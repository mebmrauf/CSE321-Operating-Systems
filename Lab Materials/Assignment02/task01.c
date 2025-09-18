#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct FibArgs { int n; };
struct SearchArgs { int *fibArray, *searchIndex, numOfSearch, n; };

void *fibonacci(void *arg);
void *search(void *arg);

int main(void) {
    int n, numOfSearch;

    printf("Enter the term of fibonacci sequence:\n");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }
    if (n < 0 || n > 40) {
        printf("n must be >= 0 and <= 40.\n");
        return 1;
    }

    printf("How many numbers you are willing to search?:\n");
    if (scanf("%d", &numOfSearch) != 1 || numOfSearch <= 0) {
        printf("Number of searches must be > 0.\n");
        return 1;
    }

    int *searchIndex = (int *)malloc(sizeof(int) * numOfSearch);
    if (!searchIndex) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < numOfSearch; ++i) {
        printf("Enter search %d:\n", i + 1);
        if (scanf("%d", &searchIndex[i]) != 1) {
            printf("Invalid search index input.\n");
            free(searchIndex);
            return 1;
        }
    }

    pthread_t t1;
    struct FibArgs fa;
    fa.n = n;
    if (pthread_create(&t1, NULL, fibonacci, &fa) != 0) {
        free(searchIndex);
        return 1;
    }

    void *fibResult = NULL;
    if (pthread_join(t1, &fibResult) != 0) {
        free(searchIndex);
        return 1;
    }
    if (fibResult == NULL) {
        free(searchIndex);
        return 1;
    }

    int *fibArray = (int *)fibResult;
    for (int i = 0; i <= n; ++i) {
        printf("a[%d] = %d\n", i, fibArray[i]);
    }

    pthread_t t2;
    struct SearchArgs sa;
    sa.fibArray = fibArray;
    sa.searchIndex = searchIndex;
    sa.numOfSearch = numOfSearch;
    sa.n = n;
    if (pthread_create(&t2, NULL, search, &sa) != 0) {
        free(fibArray);
        free(searchIndex);
        return 1;
    }

    void *res = NULL;
    if (pthread_join(t2, &res) != 0) {
        free(fibArray);
        free(searchIndex);
        return 1;
    }
    if (res == NULL) {
        free(fibArray);
        free(searchIndex);
        return 1;
    }

    int *results = (int *)res;
    for (int i = 0; i < numOfSearch; ++i) {
        printf("result of search #%d = %d\n", i + 1, results[i]);
    }

    free(results);
    free(fibArray);
    free(searchIndex);
    return 0;
}

void *fibonacci(void *arg) {
    struct FibArgs in = *(struct FibArgs *)arg;
    int n = in.n;
    int *arr = (int *)malloc(sizeof(int) * (n + 1));
    if (!arr) {
        pthread_exit(NULL);
    }
    if (n >= 0) arr[0] = 0;
    if (n >= 1) arr[1] = 1;
    for (int i = 2; i <= n; ++i) {
        arr[i] = arr[i - 1] + arr[i - 2];
    }
    pthread_exit((void *)arr);
}

void *search(void *arg) {
    struct SearchArgs in = *(struct SearchArgs *)arg;
    int *ans = malloc(sizeof(*ans) * (size_t)in.numOfSearch);
    if (!ans) pthread_exit(NULL);

    for (int i = 0; i < in.numOfSearch; ++i) {
        int idx = in.searchIndex[i];
        ans[i] = (idx >= 0 && idx <= in.n) ? in.fibArray[idx] : -1;
    }
    pthread_exit(ans);
}