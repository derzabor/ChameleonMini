/*
The DESFire stack portion of this firmware source
is free software written by Maxie Dion Schmidt (@maxieds):
You can redistribute it and/or modify
it under the terms of this license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

The complete source distribution of
this firmware is available at the following link:
https://github.com/maxieds/ChameleonMiniFirmwareDESFireStack.

Based in part on the original DESFire code created by
@dev-zzo (GitHub handle) [Dmitry Janushkevich] available at
https://github.com/dev-zzo/ChameleonMini/tree/desfire.

This notice must be retained at the top of all source files where indicated.
*/

/*
 * DESFireMemoryOperations.c
 * Maxie D. Schmidt (github.com/maxieds)
 */

#ifdef CONFIG_MF_DESFIRE_SUPPORT

#include "../../Common.h"
#include "../../Memory.h"

#include "DESFireMemoryOperations.h"
#include "DESFirePICCControl.h"
#include "DESFireFile.h"
#include "DESFireLogging.h"

#define BLOCKWISE_IO_MULTIPLIER           (DESFIRE_BLOCK_SIZE)

volatile char __InternalStringBuffer[STRING_BUFFER_SIZE] = { 0 };
char __InternalStringBuffer2[DATA_BUFFER_SIZE_SMALL] = { 0 };

void ReadBlockBytes(void *Buffer, SIZET StartBlock, SIZET Count) {
    if (StartBlock * BLOCKWISE_IO_MULTIPLIER >= MEMORY_SIZE_PER_SETTING) {
        const char *rbbLogMsg = PSTR("RBB Too Lg - %d / %d");
        DEBUG_PRINT_P(rbbLogMsg, StartBlock, MEMORY_SIZE_PER_SETTING);
        return;
    }
    MemoryDownloadBlock(Buffer, StartBlock * BLOCKWISE_IO_MULTIPLIER, Count);
}

void WriteBlockBytesMain(const void *Buffer, SIZET StartBlock, SIZET Count) {
    if (StartBlock * BLOCKWISE_IO_MULTIPLIER >= MEMORY_SIZE_PER_SETTING) {
        const char *wbbLogMsg = PSTR("WBB Too Lg - %d / %d");
        DEBUG_PRINT_P(wbbLogMsg, StartBlock, MEMORY_SIZE_PER_SETTING);
        return;
    }
    MemoryUploadBlock(Buffer, StartBlock * BLOCKWISE_IO_MULTIPLIER, Count);
}

uint16_t AllocateBlocksMain(uint16_t BlockCount) {
    uint16_t Block;
    /* Check if we have space */
    Block = Picc.FirstFreeBlock;
    if (Block + BlockCount < Block || Block + BlockCount >= MEMORY_SIZE_PER_SETTING / BLOCKWISE_IO_MULTIPLIER) {
        return 0;
    }
    Picc.FirstFreeBlock = Block + BlockCount;
    DESFIRE_FIRST_FREE_BLOCK_ID = Picc.FirstFreeBlock;
    SynchronizePICCInfo();
    return Block;
}

uint8_t GetCardCapacityBlocks(void) {
    uint8_t MaxFreeBlocks;

    switch (Picc.StorageSize) {
        case DESFIRE_STORAGE_SIZE_2K:
            MaxFreeBlocks = 0x40 - DESFIRE_FIRST_FREE_BLOCK_ID;
            break;
        case DESFIRE_STORAGE_SIZE_4K:
            MaxFreeBlocks = 0x80 - DESFIRE_FIRST_FREE_BLOCK_ID;
            break;
        case DESFIRE_STORAGE_SIZE_8K:
            MaxFreeBlocks = (BYTE)(0x100 - DESFIRE_FIRST_FREE_BLOCK_ID);
            break;
        default:
            return 0;
    }
    return MaxFreeBlocks - Picc.FirstFreeBlock;
}

void ReadDataEEPROMSource(uint8_t *Buffer, uint8_t Count) {
    MemoryReadBlock(Buffer, TransferState.ReadData.Source.Pointer, Count);
    TransferState.ReadData.Source.Pointer += DESFIRE_BYTES_TO_BLOCKS(Count);
}

void WriteDataEEPROMSink(uint8_t *Buffer, uint8_t Count) {
    MemoryWriteBlock(Buffer, TransferState.WriteData.Sink.Pointer, Count);
    TransferState.WriteData.Sink.Pointer += DESFIRE_BYTES_TO_BLOCKS(Count);
}

#endif /* CONFIG_MF_DESFIRE_SUPPORT */
