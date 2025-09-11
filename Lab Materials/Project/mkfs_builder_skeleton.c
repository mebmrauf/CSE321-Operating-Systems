// Build: gcc -O2 -std=c17 -Wall -Wextra mkfs_builder.c -o mkfs_builder
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#define BS 4096u               // block size
#define INODE_SIZE 128u
#define ROOT_INO 1u

uint64_t g_random_seed = 0; // This should be replaced by seed value from the CLI.

// below contains some basic structures you need for your project
// you are free to create more structures as you require

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;               // 0x4D565346
    uint32_t version;             // 1
    uint32_t block_size;          // 4096
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
    uint64_t root_inode;          // 1
    uint64_t mtime_epoch;
    uint32_t flags;               // 0
    uint32_t checksum;            // crc32(superblock[0..4091])
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
    uint64_t inode_crc;   // low 4 bytes store crc32 of bytes [0..119]; high 4 bytes 0

} inode_t;
#pragma pack(pop)
_Static_assert(sizeof(inode_t)==INODE_SIZE, "inode size mismatch");

#pragma pack(push,1)
typedef struct {
    uint32_t inode_no;
    uint8_t type;
    char name[58];
    uint8_t checksum; // XOR of bytes 0..62
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

// Modify your argument parsing section in main():

int main(int argc, char **argv)
{
    crc32_init();
    // Parse CLI arguments
    char *image_name = NULL;
    uint64_t size_kib = 0;
    uint64_t inode_count = 0;
    uint64_t seed_value = 0;  // Add this line
    
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--image") && i + 1 < argc) {
            image_name = argv[++i];
        } else if (!strcmp(argv[i], "--size-kib") && i + 1 < argc) {
            size_kib = strtoull(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--inodes") && i + 1 < argc) {
            inode_count = strtoull(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--seed") && i + 1 < argc) {  // Add this block
            seed_value = strtoull(argv[++i], NULL, 10);
        }
    }
    
    // Update the global variable
    g_random_seed = seed_value;  // Add this line
    
    if (!image_name || size_kib < 180 || size_kib > 4096 || size_kib % 4 != 0 || inode_count < 128 || inode_count > 512) {
        fprintf(stderr, "Invalid arguments.\n");
        fprintf(stderr, "Usage: %s --image <file> --size-kib <size> --inodes <count> [--seed <value>]\n", argv[0]);
        return 1;
    }
    
    // Optional: Print the seed being used for debugging
    printf("Using seed: %lu\n", g_random_seed);

    uint64_t total_blocks = (size_kib * 1024) / BS;
    uint64_t inode_bitmap_start = 1;
    uint64_t inode_bitmap_blocks = 1;
    uint64_t data_bitmap_start = 2;
    uint64_t data_bitmap_blocks = 1;
    uint64_t inode_table_start = 3;
    uint64_t inode_table_blocks = (inode_count * INODE_SIZE + BS - 1) / BS;
    uint64_t data_region_start = inode_table_start + inode_table_blocks;
    uint64_t data_region_blocks = total_blocks - data_region_start;
    
    // Check if there's at least one data block for root directory
    if (data_region_blocks < 1) {
        fprintf(stderr, "Not enough space for data region.\n");
        return 1;
    }
    
    // Superblock
    superblock_t superblock = {0};
    superblock.magic = 0x4D565346;
    superblock.version = 1;
    superblock.block_size = BS;
    superblock.total_blocks = total_blocks;
    superblock.inode_count = inode_count;
    superblock.inode_bitmap_start = inode_bitmap_start;
    superblock.inode_bitmap_blocks = inode_bitmap_blocks;
    superblock.data_bitmap_start = data_bitmap_start;
    superblock.data_bitmap_blocks = data_bitmap_blocks;
    superblock.inode_table_start = inode_table_start;
    superblock.inode_table_blocks = inode_table_blocks;
    superblock.data_region_start = data_region_start;
    superblock.data_region_blocks = data_region_blocks;
    superblock.root_inode = ROOT_INO;
    superblock.mtime_epoch = time(NULL);
    superblock.flags = 0;
    superblock_crc_finalize(&superblock);
    
    // Bitmaps
    uint8_t *inode_bitmap = calloc(BS, 1);
    uint8_t *data_bitmap = calloc(BS, 1);
    inode_bitmap[0] |= 1; // root inode allocated
    data_bitmap[0] |= 1; // first data block allocated for root dir
    
    // Inode table
    inode_t *inode_table = calloc(inode_count, sizeof(inode_t));
    time_t now = time(NULL);
    inode_t *root_inode = &inode_table[0];
    root_inode->mode = 040000; // directory (octal)
    root_inode->links = 2; // . and ..
    root_inode->uid = 0;
    root_inode->gid = 0;
    root_inode->size_bytes = 2 * sizeof(dirent64_t); // . and ..
    root_inode->atime = now;
    root_inode->mtime = now;
    root_inode->ctime = now;
    root_inode->direct[0] = 1; // first data block (1-based indexing for data region)
    for (int i = 1; i < 12; ++i) root_inode->direct[i] = 0;
    root_inode->reserved_0 = 0;
    root_inode->reserved_1 = 0;
    root_inode->reserved_2 = 0;
    root_inode->proj_id = 7;
    root_inode->uid16_gid16 = 0;
    root_inode->xattr_ptr = 0;
    inode_crc_finalize(root_inode);
    
    // Data region - need to allocate the full data region
    uint64_t data_region_bytes = data_region_blocks * BS;
    uint8_t *data_region = calloc(data_region_bytes, 1);
    
    // Initialize root directory entries in first block of data region
    dirent64_t *de = (dirent64_t *)data_region;
    // . entry
    de[0].inode_no = ROOT_INO;
    de[0].type = 2;
    strncpy(de[0].name, ".", 58);
    dirent_checksum_finalize(&de[0]);
    // .. entry
    de[1].inode_no = ROOT_INO;
    de[1].type = 2;
    strncpy(de[1].name, "..", 58);
    dirent_checksum_finalize(&de[1]);
    
    // Write to image
    FILE *f = fopen(image_name, "wb");
    if (!f) {
        perror("fopen");
        free(inode_bitmap);
        free(data_bitmap);
        free(inode_table);
        free(data_region);
        return 1;
    }
    
    // Write superblock (padded to full block)
    uint8_t *superblock_block = calloc(BS, 1);
    memcpy(superblock_block, &superblock, sizeof(superblock_t));
    fwrite(superblock_block, 1, BS, f);
    free(superblock_block);
    
    // Write inode bitmap block
    fwrite(inode_bitmap, 1, BS, f);
    
    // Write data bitmap block
    fwrite(data_bitmap, 1, BS, f);
    
    // Write inode table
    for (uint64_t i = 0; i < inode_count; ++i) {
        fwrite(&inode_table[i], 1, sizeof(inode_t), f);
    }
    // Pad inode table to block size
    uint64_t inode_table_bytes = inode_count * sizeof(inode_t);
    uint64_t inode_table_pad = inode_table_blocks * BS - inode_table_bytes;
    if (inode_table_pad) {
        uint8_t *pad = calloc(inode_table_pad, 1);
        fwrite(pad, 1, inode_table_pad, f);
        free(pad);
    }
    
    // Write entire data region
    fwrite(data_region, 1, data_region_bytes, f);
    
    fclose(f);
    free(inode_bitmap);
    free(data_bitmap);
    free(inode_table);
    free(data_region);
    return 0;
}