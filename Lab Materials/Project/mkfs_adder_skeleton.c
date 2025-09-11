#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdint.h>
#include <string.h>    // For memcpy, memset, strcmp, strncpy
#include <stdlib.h>    // For calloc, free
#include <time.h>      // For time_t and time()
#include <stdbool.h>   // For bool, true, false

#define BS 4096u
#define INODE_SIZE 128u
#define ROOT_INO 1u
#define DIRECT_MAX 12
#pragma pack(push, 1)

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint64_t total_blocks;
    uint64_t inode_count;
    uint64_t inode_bitmap_start;
    uint64_t inode_bitmap_blocks;
    uint64_t data_bitmap_start;
    uint64_t data_bitmap_blocks;
    uint64_t inode_table_start;
    uint64_t inode_table_blocks;
    uint64_t data_region_start;
    uint64_t data_region_blocks;
    uint64_t root_inode;
    uint64_t mtime_epoch;
    uint32_t flags;
    uint32_t checksum;
} superblock_t;
#pragma pack(pop)
_Static_assert(sizeof(superblock_t) == 116, "superblock must fit in one block");

#pragma pack(push,1)
typedef struct {
    uint16_t mode;
    uint16_t links;
    uint32_t uid;
    uint32_t gid;
    uint64_t size_bytes;
    uint64_t atime;
    uint64_t mtime;
    uint64_t ctime;
    uint32_t direct[12];
    uint32_t reserved_0;
    uint32_t reserved_1;
    uint32_t reserved_2;
    uint32_t proj_id;
    uint32_t uid16_gid16;
    uint64_t xattr_ptr;
    uint64_t inode_crc;

} inode_t;
#pragma pack(pop)
_Static_assert(sizeof(inode_t)==INODE_SIZE, "inode size mismatch");

#pragma pack(push,1)
typedef struct {
    uint32_t inode_no;
    uint8_t type;
    char name[58];
    uint8_t checksum;
} dirent64_t;
#pragma pack(pop)
_Static_assert(sizeof(dirent64_t)==64, "dirent size mismatch");


// ==========================DO NOT CHANGE THIS PORTION=========================
// These functions are there for your help. You should refer to the specifications to see how you can use them.
// ====================================CRC32====================================
uint32_t CRC32_TAB[256];
void crc32_init(void){
    for (uint32_t i=0;i<256;i++){
        uint32_t c=i;
        for(int j=0;j<8;j++) c = (c&1)?(0xEDB88320u^(c>>1)):(c>>1);
        CRC32_TAB[i]=c;
    }
}
uint32_t crc32(const void* data, size_t n){
    const uint8_t* p=(const uint8_t*)data; uint32_t c=0xFFFFFFFFu;
    for(size_t i=0;i<n;i++) c = CRC32_TAB[(c^p[i])&0xFF] ^ (c>>8);
    return c ^ 0xFFFFFFFFu;
}
// ====================================CRC32====================================

// WARNING: CALL THIS ONLY AFTER ALL OTHER SUPERBLOCK ELEMENTS HAVE BEEN FINALIZED
static uint32_t superblock_crc_finalize(superblock_t *sb) {
    sb->checksum = 0;
    uint32_t s = crc32((void *) sb, BS - 4);
    sb->checksum = s;
    return s;
}

// WARNING: CALL THIS ONLY AFTER ALL OTHER SUPERBLOCK ELEMENTS HAVE BEEN FINALIZED
void inode_crc_finalize(inode_t* ino){
    uint8_t tmp[INODE_SIZE]; memcpy(tmp, ino, INODE_SIZE);
    // zero crc area before computing
    memset(&tmp[120], 0, 8);
    uint32_t c = crc32(tmp, 120);
    ino->inode_crc = (uint64_t)c; // low 4 bytes carry the crc
}

// WARNING: CALL THIS ONLY AFTER ALL OTHER SUPERBLOCK ELEMENTS HAVE BEEN FINALIZED
void dirent_checksum_finalize(dirent64_t* de) {
    const uint8_t* p = (const uint8_t*)de;
    uint8_t x = 0;
    for (int i = 0; i < 63; i++) x ^= p[i];   // covers ino(4) + type(1) + name(58)
    de->checksum = x;
}

int main(int argc, char **argv) {
    crc32_init();
    // Parse CLI arguments
    char *input_img = NULL, *output_img = NULL, *add_file = NULL;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--input") && i + 1 < argc) {
            input_img = argv[++i];
        } else if (!strcmp(argv[i], "--output") && i + 1 < argc) {
            output_img = argv[++i];
        } else if (!strcmp(argv[i], "--file") && i + 1 < argc) {
            add_file = argv[++i];
        }
    }
    if (!input_img || !output_img || !add_file) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }
    
    // Open input image
    FILE *fin = fopen(input_img, "rb");
    if (!fin) { 
        perror("fopen input"); 
        return 1; 
    }
    
    // Read and validate superblock
    superblock_t superblock;
    if (fread(&superblock, 1, sizeof(superblock_t), fin) != sizeof(superblock_t)) {
        fprintf(stderr, "Failed to read superblock.\n");
        fclose(fin);
        return 1;
    }
    
    if (superblock.magic != 0x4D565346 || superblock.block_size != BS) {
        fprintf(stderr, "Invalid image format.\n");
        fclose(fin); 
        return 1;
    }
    
    // Read bitmaps
    fseek(fin, superblock.inode_bitmap_start * BS, SEEK_SET);
    uint8_t *inode_bitmap = calloc(BS, 1);
    if (!inode_bitmap || fread(inode_bitmap, 1, BS, fin) != BS) {
        fprintf(stderr, "Failed to read inode bitmap.\n");
        free(inode_bitmap);
        fclose(fin);
        return 1;
    }
    
    fseek(fin, superblock.data_bitmap_start * BS, SEEK_SET);
    uint8_t *data_bitmap = calloc(BS, 1);
    if (!data_bitmap || fread(data_bitmap, 1, BS, fin) != BS) {
        fprintf(stderr, "Failed to read data bitmap.\n");
        free(inode_bitmap);
        free(data_bitmap);
        fclose(fin);
        return 1;
    }
    
    // Read inode table
    fseek(fin, superblock.inode_table_start * BS, SEEK_SET);
    inode_t *inode_table = calloc(superblock.inode_count, sizeof(inode_t));
    if (!inode_table || fread(inode_table, sizeof(inode_t), superblock.inode_count, fin) != superblock.inode_count) {
        fprintf(stderr, "Failed to read inode table.\n");
        free(inode_bitmap);
        free(data_bitmap);
        free(inode_table);
        fclose(fin);
        return 1;
    }
    
    // Read data region
    fseek(fin, superblock.data_region_start * BS, SEEK_SET);
    uint64_t data_region_bytes = superblock.data_region_blocks * BS;
    uint8_t *data_region = calloc(data_region_bytes, 1);
    if (!data_region || fread(data_region, 1, data_region_bytes, fin) != data_region_bytes) {
        fprintf(stderr, "Failed to read data region.\n");
        free(inode_bitmap);
        free(data_bitmap);
        free(inode_table);
        free(data_region);
        fclose(fin);
        return 1;
    }
    fclose(fin);
    
    // ==================== Duplicate Filename Check ====================
    inode_t *root_inode = &inode_table[0]; // Root inode is at index 0 (1-indexed)
    int name_exists = 0;
    for (int i = 0; i < DIRECT_MAX && root_inode->direct[i]; ++i) {
        uint32_t blk = root_inode->direct[i];
        dirent64_t *block_dir = (dirent64_t*)(data_region + (blk - 1) * BS);
        for (int j = 0; j < (int)(BS / sizeof(dirent64_t)); ++j) {
            if (block_dir[j].inode_no != 0) {
                if (strncmp(block_dir[j].name, add_file, sizeof(block_dir[j].name)) == 0) {
                    fprintf(stderr, "Error: File with name '%s' already exists in root directory.\n", add_file);
                    name_exists = 1;
                    break;
                }
            }
        }
        if (name_exists) break;
    }
    if (name_exists) {
        goto cleanup; // Terminate without writing new file
    }
    
    // Find file to add
    FILE *ffile = fopen(add_file, "rb");
    if (!ffile) { 
        fprintf(stderr, "File to add not found.\n"); 
        goto cleanup; 
    }
    
    fseek(ffile, 0, SEEK_END);
    uint64_t file_size = ftell(ffile);
    fseek(ffile, 0, SEEK_SET);
    
    // Check if file fits in 12 direct blocks
    uint64_t blocks_needed = (file_size + BS - 1) / BS;
    if (blocks_needed > DIRECT_MAX) {
        fprintf(stderr, "File too large for MiniVSFS.\n"); 
        fclose(ffile); 
        goto cleanup;
    }
    
    // Find free inode
    int free_ino = -1;
    for (uint64_t i = 0; i < superblock.inode_count; ++i) {
        if (!(inode_bitmap[i / 8] & (1 << (i % 8)))) { 
            free_ino = i; 
            break; 
        }
    }
    if (free_ino == -1) {
        fprintf(stderr, "No free inode available.\n"); 
        fclose(ffile); 
        goto cleanup;
    }
    
    // Find free data blocks
    int free_blocks[DIRECT_MAX] = {0};
    int found_blocks = 0;
    for (uint64_t i = 0; i < superblock.data_region_blocks && found_blocks < (int)blocks_needed; ++i) {
        if (!(data_bitmap[i / 8] & (1 << (i % 8)))) {
            free_blocks[found_blocks++] = i;
        }
    }
    if (found_blocks < (int)blocks_needed) {
        fprintf(stderr, "Not enough free data blocks.\n"); 
        fclose(ffile); 
        goto cleanup;
    }
    
    // Allocate inode and data blocks
    inode_bitmap[free_ino / 8] |= (1 << (free_ino % 8));
    for (int i = 0; i < (int)blocks_needed; ++i) {
        data_bitmap[free_blocks[i] / 8] |= (1 << (free_blocks[i] % 8));
    }
    
    // Write file data to data blocks
    for (int i = 0; i < (int)blocks_needed; ++i) {
        size_t bytes_to_read = BS;
        if (i == (int)blocks_needed - 1 && file_size > 0) {
            // Last block - only read remaining bytes
            bytes_to_read = file_size % BS;
            if (bytes_to_read == 0) bytes_to_read = BS; // Full last block
        }
        
        // Clear the block first (important for partial blocks)
        memset(data_region + free_blocks[i] * BS, 0, BS);
        
        if (fread(data_region + free_blocks[i] * BS, 1, bytes_to_read, ffile) != bytes_to_read) {
            fprintf(stderr, "Failed to read file data.\n");
            fclose(ffile);
            goto cleanup;
        }
    }
    fclose(ffile);
    
    // Update inode table
    inode_t *new_inode = &inode_table[free_ino];
    memset(new_inode, 0, sizeof(inode_t)); // Clear the inode first
    new_inode->mode = 0100000; // file (octal)
    new_inode->links = 1;
    new_inode->uid = 0;
    new_inode->gid = 0;
    new_inode->size_bytes = file_size;
    time_t now = time(NULL);
    new_inode->atime = now;
    new_inode->mtime = now;
    new_inode->ctime = now;
    for (int i = 0; i < DIRECT_MAX; ++i) {
        new_inode->direct[i] = (i < (int)blocks_needed) ? (free_blocks[i] + 1) : 0;
    }
    new_inode->reserved_0 = 0;
    new_inode->reserved_1 = 0;
    new_inode->reserved_2 = 0;
    new_inode->proj_id = 7;
    new_inode->uid16_gid16 = 0;
    new_inode->xattr_ptr = 0;
    inode_crc_finalize(new_inode);
    
    // Update root directory
    // Find a free directory entry across all allocated blocks
    int dirent_idx = -1;
    dirent64_t *target_dir = NULL;
    bool allocated_new_block = false;

    // Check existing allocated blocks first
    for (int b = 0; b < 12 && root_inode->direct[b] != 0; ++b) {
        uint32_t root_data_block = root_inode->direct[b];
        dirent64_t *root_dir = (dirent64_t *)(data_region + (root_data_block - 1) * BS);
        
        for (int i = 0; i < (int)(BS / sizeof(dirent64_t)); ++i) {
            if (root_dir[i].inode_no == 0) {
                dirent_idx = i;
                target_dir = root_dir;
                break;
            }
        }
        if (dirent_idx != -1) break;
    }

    // If no free entry found in existing blocks, allocate a new block
    if (dirent_idx == -1) {
        // Find next available direct block slot
        int next_direct_idx = -1;
        for (int i = 0; i < 12; ++i) {
            if (root_inode->direct[i] == 0) {
                next_direct_idx = i;
                break;
            }
        }
        
        if (next_direct_idx == -1) {
            fprintf(stderr, "Root directory has reached maximum size (12 direct blocks).\n");
            goto cleanup;
        }
        
        // Find a free data block
        int new_data_block = -1;
        for (uint64_t i = 0; i < superblock.data_region_blocks; ++i) {
            if (!(data_bitmap[i / 8] & (1 << (i % 8)))) {
                new_data_block = i;
                break;
            }
        }
        
        if (new_data_block == -1) {
            fprintf(stderr, "No free data blocks available for root directory expansion.\n");
            goto cleanup;
        }
        
        // Allocate the new data block
        data_bitmap[new_data_block / 8] |= (1 << (new_data_block % 8));
        
        // Update root inode to point to new block
        root_inode->direct[next_direct_idx] = new_data_block + 1; // 1-based indexing
        
        // Clear the new block (it should already be zero, but be explicit)
        memset(data_region + new_data_block * BS, 0, BS);
        
        // Set up for adding entry to the new block
        target_dir = (dirent64_t *)(data_region + new_data_block * BS);
        dirent_idx = 0; // First entry in the new block
        allocated_new_block = true;
    }

    // Add new directory entry
    target_dir[dirent_idx].inode_no = free_ino + 1; // 1-based indexing
    target_dir[dirent_idx].type = 1; // file
    strncpy(target_dir[dirent_idx].name, add_file, 58);
    target_dir[dirent_idx].name[57] = '\0'; // Ensure null termination
    dirent_checksum_finalize(&target_dir[dirent_idx]);

    // Update root inode metadata
    // Calculate current directory size properly
    int allocated_blocks = 0;
    for (int i = 0; i < 12; ++i) {
        if (root_inode->direct[i] != 0) {
            allocated_blocks++;
        }
    }
    
    // Set size to the total allocated space for the directory
    root_inode->size_bytes = allocated_blocks * BS;
    root_inode->mtime = now;
    root_inode->ctime = now; // Also update ctime since directory structure changed
    inode_crc_finalize(root_inode);
    
    // Write updated image
    FILE *fout = fopen(output_img, "wb");
    if (!fout) { 
        perror("fopen output"); 
        goto cleanup; 
    }
    
    // Update and write superblock
    superblock.mtime_epoch = now;
    superblock_crc_finalize(&superblock);
    
    // Write superblock (padded to full block)
    uint8_t *superblock_block = calloc(BS, 1);
    if (!superblock_block) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(fout);
        goto cleanup;
    }
    memcpy(superblock_block, &superblock, sizeof(superblock_t));
    fwrite(superblock_block, 1, BS, fout);
    free(superblock_block);
    
    // Write bitmaps
    fwrite(inode_bitmap, 1, BS, fout);
    fwrite(data_bitmap, 1, BS, fout);
    
    // Write inode table
    for (uint64_t i = 0; i < superblock.inode_count; ++i) {
        fwrite(&inode_table[i], 1, sizeof(inode_t), fout);
    }
    // Pad inode table to block size
    uint64_t inode_table_bytes = superblock.inode_count * sizeof(inode_t);
    uint64_t inode_table_pad = superblock.inode_table_blocks * BS - inode_table_bytes;
    if (inode_table_pad) {
        uint8_t *pad = calloc(inode_table_pad, 1);
        if (pad) {
            fwrite(pad, 1, inode_table_pad, fout);
            free(pad);
        }
    }
    
    // Write data region
    fwrite(data_region, 1, data_region_bytes, fout);
    fclose(fout);
    
    printf("Successfully added file '%s' to filesystem.\n", add_file);
    
cleanup:
    free(inode_bitmap);
    free(data_bitmap);
    free(inode_table);
    free(data_region);
    return 0;
}