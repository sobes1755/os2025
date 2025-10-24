#define SWAP(X, Y)                       \
    do {                                 \
        auto const SWAP_PTR_X = &(X);    \
        auto const SWAP_PTR_Y = &(Y);    \
        auto SWAP_TMP = *SWAP_PTR_X;     \
        *SWAP_PTR_X = *SWAP_PTR_Y;       \
        *SWAP_PTR_Y = SWAP_TMP;          \
    } while (false)
