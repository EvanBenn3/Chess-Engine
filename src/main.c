#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef unsigned long long int u64;
typedef unsigned char u8;

typedef struct {
    u64 whitePawns;
    u64 whiteKnights;
    u64 whiteBishops;
    u64 whiteRooks;
    u64 whiteQueens;
    u64 whiteKing;
    u64 blackPawns;
    u64 blackKnights;
    u64 blackBishops;
    u64 blackRooks;
    u64 blackQueens;
    u64 blackKing;

    bool whitesTurn;
    u8 castelingRights;
    u8 enPassantSquare;
    u8 halfmoveClock;
    u8 fullmoveNumber;
} Board;

typedef struct {
    u8 from;
    u8 to;
    u8 prom;
    bool isCapture;
} Move;

u8 my_abs(char n) {
    return n < 0 ? -n : n;
}

int my_strtok(char ***tokens, const char *string, char del) {
    int ptr = 0;
    int strptr = 0;
    int tokptr = 0;
    (*tokens) = realloc((*tokens), sizeof(char*));
    (*tokens)[tokptr] = NULL;
    while (string[ptr] != 0) {
        if (string[ptr] == del) {
            (*tokens)[tokptr] = realloc((*tokens)[tokptr], (strptr+1)*sizeof(char));
            (*tokens)[tokptr][strptr] = 0;
            tokptr++;
            (*tokens) = realloc((*tokens), (tokptr+1)*sizeof(char*));
            (*tokens)[tokptr] = NULL;
            strptr = 0;
        } else {
            (*tokens)[tokptr] = realloc((*tokens)[tokptr], (strptr+1)*sizeof(char));
            (*tokens)[tokptr][strptr] = string[ptr];
            strptr++;
        }
        ptr++;
    }
    (*tokens)[tokptr] = realloc((*tokens)[tokptr], (strptr+1)*sizeof(char));
    (*tokens)[tokptr][strptr] = 0;
    return ++tokptr;
}

bool my_strcmp(const char *string1, const char *string2) {
    int ptr = 0;
    while (string1[ptr] != 0 || string2[ptr] != 0) {
        if (string1[ptr] != string2[ptr]) {
            return false;
        }
        ptr++;
    }
    return true;
}

int my_strlen(const char* string) {
    int n = 0;
    while (string[n] != '\0') n++;
    return n;
}

void initBoard(Board* board) {
    board->blackPawns =   0;
    board->blackKnights = 0;
    board->blackBishops = 0;
    board->blackRooks =   0;
    board->blackQueens =  0;
    board->blackKing =    0;
    board->whitePawns =   0;
    board->whiteKnights = 0;
    board->whiteBishops = 0;
    board->whiteRooks =   0;
    board->whiteQueens =  0;
    board->whiteKing =    0;
}

void blockCheckWhite(Board board, u64* blockMap) {
    (*blockMap) = board.whitePawns | board.whiteBishops | board.whiteKnights | board.whiteQueens | board.whiteRooks | board.whiteKing;
}

void blockCheckBlack(Board board, u64* blockMap) {
    (*blockMap) = board.blackPawns | board.blackBishops | board.blackKnights | board.blackQueens | board.blackRooks | board.blackKing;
}

void addMove(Move* moveList, int* movelistCounter, u8 to, u8 from, u8 prom, bool isCapture) {
    moveList[*movelistCounter].to = to;
    moveList[*movelistCounter].from = from;
    moveList[*movelistCounter].prom = prom;
    moveList[(*movelistCounter)++].isCapture = isCapture;
}

void pruneMoves(Move* moveList, int* movelistCounter) {
    int ogSize = *movelistCounter;
    int toPrune[64];
    int toPruneSize = 0;
    for (int i = 0; i < ogSize; i++) {
        if (moveList[i].to == 0xff) {
            toPrune[toPruneSize++] = i;
        }
    }

    for (int i = 0; i < toPruneSize; i++) {
        for (int j = toPrune[i]; j < (ogSize-1); j++) {
            moveList[j] = moveList[j+1];
        }
        ogSize--;
    }

    (*movelistCounter) = ogSize;
}

void generatePseudoWhiteMoves(Board board, Move* moveList, int* movelistCounter) {
    Board temp = board;
    u64 blockMapWhite;
    u64 blockMapBlack;
    u64 blockMap;

    u8 pawnList[8];
    int pawnListSize = 0;
    u8 knightList[10];
    int knightListSize = 0;
    u8 bishopList[10];
    int bishopListSize = 0;
    u8 rookList[10];
    int rookListSize = 0;
    u8 queenList[9];
    int queenListSize = 0;
    u8 kingpos;

    for (int i = 0; i < 8; i++) {
        if (((temp.whitePawns >> i*8) & 0xff) > 0) {
            for (int j = 0; j < 8; j++) {
                if (((temp.whitePawns >> i*8+j) & 0x1) == 1) {
                    temp.whitePawns &= ~(1ULL << i*8+j);
                    pawnList[pawnListSize++] = i*8+j;
                }
            }
        }
    }
    
    blockCheckWhite(board, &blockMapWhite);
    blockCheckBlack(board, &blockMapBlack);
    blockMap = blockMapWhite | blockMapBlack;

    if (board.enPassantSquare != 0xff) {
        blockMapBlack |= (1ULL << board.enPassantSquare);
    }
    
    for (int i = 0; i < pawnListSize; i++) {
        u8 src = pawnList[i];
        if (((1ULL << (src + 8)) & blockMap) == 0) {
            addMove(moveList, movelistCounter, src+8, src, 0, false);
        }
        if (((((1ULL << (src + 16)) | (1ULL << (src + 8))) & blockMap) == 0) && ((src / 8) == 1)) {
            addMove(moveList, movelistCounter, src+16, src, 0, false);
        }
        if ((src & 0x7) != 0) {
            if ((1ULL << (src + 7)) & blockMapBlack) {
                addMove(moveList, movelistCounter, src+7, src, 0, true);
            }
        }
        if ((src & 0x7) != 0x7) {
            if ((1ULL << (src + 9)) & blockMapBlack) {
                addMove(moveList, movelistCounter, src+9, src, 0, true);
            }
        }
    }

    if (board.enPassantSquare != 0xff) {
        blockMapBlack &= ~(1ULL << board.enPassantSquare);
    }

    pawnListSize = *movelistCounter;
    for (int i = 0; i < pawnListSize; i++) {
        if ((moveList[i].to / 8) == 7) {
            moveList[i].prom = 'Q';
            
            addMove(moveList, movelistCounter, moveList[i].to, moveList[i].from, 'R', moveList[i].isCapture);
            addMove(moveList, movelistCounter, moveList[i].to, moveList[i].from, 'B', moveList[i].isCapture);
            addMove(moveList, movelistCounter, moveList[i].to, moveList[i].from, 'N', moveList[i].isCapture);
        }
    }

    temp = board;
    for (int i = 0; i < 8; i++) {
        if (((temp.whiteKnights >> i*8) & 0xff) > 0) {
            for (int j = 0; j < 8; j++) {
                if (((temp.whiteKnights >> i*8+j) & 0x1) == 1) {
                    temp.whiteKnights &= ~(1ULL << i*8+j);
                    knightList[knightListSize++] = i*8+j;
                }
            }
        }
    }

    char knightMoveset[8] = {17, 15, 10, -6, -15, -17, -10, 6};
    for (int i = 0; i < knightListSize; i++) {
        u8 src = knightList[i];
        u8 col = src & 0x7;
        u8 row = src / 8;
        for (int j = 0; j < 8; j++) {
            u8 dest = src + knightMoveset[j];
            u8 destcol = dest & 0x7;
            u8 destrow = dest / 8;
            char diffcol = my_abs(destcol - col);
            char diffrow = my_abs(destrow - row);
            if (dest < 64) {
                if ((diffcol <= 2) && (diffrow <= 2)) {
                    if ((blockMapWhite & (1ULL << dest)) == 0) {
                        if ((blockMapBlack & (1ULL << dest)) == 0) {
                            addMove(moveList, movelistCounter, dest, src, 0, false);
                        } else {
                            addMove(moveList, movelistCounter, dest, src, 0, true);
                        }
                    }
                }
            }
        }
    }

    temp = board;
    for (int i = 0; i < 8; i++) {
        if (((temp.whiteBishops >> i*8) & 0xff) > 0) {
            for (int j = 0; j < 8; j++) {
                if (((temp.whiteBishops >> i*8+j) & 0x1) == 1) {
                    temp.whiteBishops &= ~(1ULL << i*8+j);
                    bishopList[bishopListSize++] = i*8+j;
                }
            }
        }
    }

    char bishopMoveset[4] = {7, 9, -7, -9};
    for (int i = 0; i < bishopListSize; i++) {
        u8 src = bishopList[i];
        for (int j = 0; j < 4; j++) {
            bool run = true;
            u8 dest = src;
            while (run) {
                u8 col = src & 0x7;
                u8 row = src / 8;
                dest += bishopMoveset[j];
                col = dest & 0x7;
                row = dest / 8;
                int srcRow = (dest - bishopMoveset[j]) / 8;
                int srcCol = (dest - bishopMoveset[j]) % 8;
                if (my_abs(col - srcCol) > 1) {
                    run = false;
                    break;
                }
                if (my_abs(row - srcRow) > 1) {
                    run = false;
                    break;
                }
                if (dest < 0 || dest >= 64) {
                    run = false;
                    break;
                }
                if ((row == 0) || (row == 7) || (col == 0) || (col == 7)) {
                    run = false;
                }
                if ((blockMap & (1ULL << dest)) > 0) {
                    run = false;
                    if ((blockMapBlack & (1ULL << dest)) > 0) {
                        addMove(moveList, movelistCounter, dest, src, 0, true);
                    }
                } else {
                    addMove(moveList, movelistCounter, dest, src, 0, false);
                }
            }
        }
    }

    temp = board;
    for (int i = 0; i < 8; i++) {
        if (((temp.whiteRooks >> i*8) & 0xff) > 0) {
            for (int j = 0; j < 8; j++) {
                if (((temp.whiteRooks >> i*8+j) & 0x1) == 1) {
                    temp.whiteRooks &= ~(1ULL << i*8+j);
                    rookList[rookListSize++] = i*8+j;
                }
            }
        }
    }

    char rookMoveset[4] = {8, 1, -8, -1};
    for (int i = 0; i < rookListSize; i++) {
        u8 src = rookList[i];
        for (int j = 0; j < 4; j++) {
            bool run = true;
            u8 dest = src;
            while (run) {
                u8 col = src & 0x7;
                u8 row = src / 8;
                dest += rookMoveset[j];
                col = dest & 0x7;
                row = dest / 8;
                int srcRow = (dest - rookMoveset[j]) / 8;
                int srcCol = (dest - rookMoveset[j]) % 8;
                if (my_abs(col - srcCol) > 1) {
                    run = false;
                    break;
                }
                if (my_abs(row - srcRow) > 1) {
                    run = false;
                    break;
                }
                if (dest < 0 || dest >= 64) {
                    run = false;
                    break;
                }
                if ((blockMap & (1ULL << dest)) > 0) {
                    run = false;
                    if ((blockMapBlack & (1ULL << dest)) > 0) {
                        addMove(moveList, movelistCounter, dest, src, 0, true);
                    }
                } else {
                    addMove(moveList, movelistCounter, dest, src, 0, false);
                }
            }
        }
    }

    temp = board;
    for (int i = 0; i < 8; i++) {
        if (((temp.whiteQueens >> i*8) & 0xff) > 0) {
            for (int j = 0; j < 8; j++) {
                if (((temp.whiteQueens >> i*8+j) & 0x1) == 1) {
                    temp.whiteQueens &= ~(1ULL << i*8+j);
                    queenList[queenListSize++] = i*8+j;
                }
            }
        }
    }

    char queenMoveset[8] = {8, 1, -8, -1, 9, 7, -9, -7};
    for (int i = 0; i < queenListSize; i++) {
        u8 src = queenList[i];
        for (int j = 0; j < 8; j++) {
            bool run = true;
            u8 dest = src;
            while (run) {
                u8 col = src & 0x7;
                u8 row = src / 8;
                dest += queenMoveset[j];
                col = dest & 0x7;
                row = dest / 8;
                int srcRow = (dest - queenMoveset[j]) / 8;
                int srcCol = (dest - queenMoveset[j]) % 8;
                if (my_abs(col - srcCol) > 1) {
                    run = false;
                    break;
                }
                if (my_abs(row - srcRow) > 1) {
                    run = false;
                    break;
                }
                if (dest < 0 || dest >= 64) {
                    run = false;
                    break;
                }
                if ((blockMap & (1ULL << dest)) > 0) {
                    run = false;
                    if ((blockMapBlack & (1ULL << dest)) > 0) {
                        addMove(moveList, movelistCounter, dest, src, 0, true);
                    }
                } else {
                    addMove(moveList, movelistCounter, dest, src, 0, false);
                }
            }
        }
    }

    temp = board;
    for (int i = 0; i < 8; i++) {
        if (((temp.whiteKing >> i*8) & 0xff) > 0) {
            for (int j = 0; j < 8; j++) {
                if (((temp.whiteKing >> i*8+j) & 0x1) == 1) {
                    temp.whiteKing &= ~(1ULL << i*8+j);
                    kingpos = i*8+j;
                    break;
                }
            }
        }
    }

    char kingMoveset[8] = {8, 1, -8, -1, 9, 7, -9, -7};
    for (int i = 0; i < 8; i++) {
        u8 col = kingpos & 0x7;
        u8 row = kingpos / 8;
        u8 dest = kingpos + kingMoveset[i];
        col = dest & 0x7;
        row = dest / 8;
        int srcRow = (dest - kingMoveset[i]) / 8;
        int srcCol = (dest - kingMoveset[i]) % 8;
        if (my_abs(col - srcCol) > 1) {
            continue;
        }
        if (my_abs(row - srcRow) > 1) {
            continue;
        }
        if (dest < 0 || dest >= 64) {
            continue;
        }
        if ((blockMap & (1ULL << dest)) > 0) {
            if ((blockMapBlack & (1ULL << dest)) > 0) {
                addMove(moveList, movelistCounter, dest, kingpos, 0, true);
            }
        } else {
            addMove(moveList, movelistCounter, dest, kingpos, 0, false);
        }
    }

    if (board.castelingRights & 0b1000) {
        if ((blockMap & ((1ULL << 5) | (1ULL << 6))) == 0) {
            addMove(moveList, movelistCounter, 6, 4, 0, false);
        }
    }
    if (board.castelingRights & 0b0100) {
        if ((blockMap & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3))) == 0) {
            addMove(moveList, movelistCounter, 2, 4, 0, false);
        }
    }

}

void generateMoves(Board board, Move* moveList, int* movelistCounter) {
    
}

void FENtoBit(Board* board, const char* FENstring) {
    char** tokens = NULL;
    int tokenSize = my_strtok(&tokens, FENstring, ' ');

    if (my_strcmp(tokens[1], "w")) {
        board->whitesTurn = true;
    } else if (my_strcmp(tokens[1], "b")) {
        board->whitesTurn = false;
    }

    int i = 0;
    while (tokens[2][i] != '\0') {
        if (tokens[2][i] == '-') {
            board->castelingRights = 0;
            break;
        } else if (tokens[2][i] == 'K') {
            board->castelingRights |= 0b1000;
        } else if (tokens[2][i] == 'Q') {
            board->castelingRights |= 0b0100;
        } else if (tokens[2][i] == 'k') {
            board->castelingRights |= 0b0010;
        } else if (tokens[2][i] == 'q') {
            board->castelingRights |= 0b0001;
        }
        i++;
    }

    if (my_strcmp(tokens[3], "-")) {
        board->enPassantSquare = 0xff;
    } else {
        if (tokens[3][0] == 'a') {
            board->enPassantSquare = 0;
        } else if (tokens[3][0] == 'b') {
            board->enPassantSquare = 1;
        } else if (tokens[3][0] == 'c') {
            board->enPassantSquare = 2;
        } else if (tokens[3][0] == 'd') {
            board->enPassantSquare = 3;
        } else if (tokens[3][0] == 'e') {
            board->enPassantSquare = 4;
        } else if (tokens[3][0] == 'f') {
            board->enPassantSquare = 5;
        } else if (tokens[3][0] == 'g') {
            board->enPassantSquare = 6;
        } else if (tokens[3][0] == 'h') {
            board->enPassantSquare = 7;
        }
        if (tokens[3][1] == '3') {
            board->enPassantSquare += 16;
        } else if (tokens[3][1] == '6') {
            board->enPassantSquare += 40;
        }
    }

    board->halfmoveClock = atoi(tokens[4]);
    board->fullmoveNumber = atoi(tokens[5]);

    initBoard(board);
    int rank = 7;
    int pos = 56;
    for (int i = 0; i < my_strlen(tokens[0]); i++) {
        if (tokens[0][i] == 'P') {
            board->whitePawns |= (1ULL << pos);
        } else if (tokens[0][i] == 'N') {
            board->whiteKnights |= (1ULL << pos);
        } else if (tokens[0][i] == 'B') {
            board->whiteBishops |= (1ULL << pos);
        } else if (tokens[0][i] == 'R') {
            board->whiteRooks |= (1ULL << pos);
        } else if (tokens[0][i] == 'Q') {
            board->whiteQueens |= (1ULL << pos);
        } else if (tokens[0][i] == 'K') {
            board->whiteKing |= (1ULL << pos);
        } else if (tokens[0][i] == 'p') {
            board->blackPawns |= (1ULL << pos);
        } else if (tokens[0][i] == 'n') {
            board->blackKnights |= (1ULL << pos);
        } else if (tokens[0][i] == 'b') {
            board->blackBishops |= (1ULL << pos);
        } else if (tokens[0][i] == 'r') {
            board->blackRooks |= (1ULL << pos);
        } else if (tokens[0][i] == 'q') {
            board->blackQueens |= (1ULL << pos);
        } else if (tokens[0][i] == 'k') {
            board->blackKing |= (1ULL << pos);
        } else if (tokens[0][i] == '/') {
            rank--;
            pos = rank*8 - 1;
        } else {
            pos += atoi(&tokens[0][i]) - 1;
        }

        pos++;
    }

    for (int i = 0; i < tokenSize; i++) {
        free(tokens[i]);
        tokens[i] = NULL;
    }
    free(tokens);
    tokens = NULL;
}

int main () {
    Move movelist[256];
    int moveListCounter = 0;

    Board board;
    char FENstring[128] = "r2qk2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b kq - 0 1";
    FENtoBit(&board, FENstring);
    
    generatePseudoWhiteMoves(board, movelist, &moveListCounter);
    for (int i = 0; i < moveListCounter; i++) {
        printf("%d, %d\n", movelist[i].from, movelist[i].to);
    }

    return 0;
}