# capture_device_state.gdb -- exfiltrate the live G&W game state, no reset.
#
# Pairs with desktop/state_inject.c (see its header for the full pipeline).
# Usage:
#   1. gnwmanager gdbserver          # in another terminal; probe attached
#   2. mkdir -p /tmp/ff4cap
#   3. arm-none-eabi-gdb -q -nx -batch -x desktop/capture_device_state.gdb \
#        <retro-go-sd>/build/gw_retro_go.elf \
#        | grep -E '^(S|C|P|D|I1|I2)->' > /tmp/ff4cap/device_state.inc
#   4. make ff4-state-inject CAPTURE_DIR=/tmp/ff4cap
#   5. ./ff4-state-inject <rom> /tmp/ff4cap <out.lss>
#
# Attach halts the game for the few seconds of the dump; `monitor resume`
# at the end resumes it exactly where it was. The ELF must match the
# flashed firmware (field offsets come from its DWARF info). The scalar
# field list below mirrors snes_handleState()'s serialization exactly --
# keep them in sync if statehandler coverage ever changes.
#
set pagination off
set confirm off
target extended-remote localhost:3333

# ---- big arrays (raw little-endian dumps, layout-safe) ----
dump binary memory /tmp/ff4cap/ram.bin      ff4_snes->ram      (ff4_snes->ram + 0x20000)
dump binary memory /tmp/ff4cap/vram.bin     ff4_snes->ppu->vram ((char*)ff4_snes->ppu->vram + 0x10000)
dump binary memory /tmp/ff4cap/cgram.bin    ff4_snes->ppu->cgram ((char*)ff4_snes->ppu->cgram + 0x200)
dump binary memory /tmp/ff4cap/oam.bin      ff4_snes->ppu->oam ((char*)ff4_snes->ppu->oam + 0x200)
dump binary memory /tmp/ff4cap/highoam.bin  ff4_snes->ppu->highOam (ff4_snes->ppu->highOam + 0x20)
dump binary memory /tmp/ff4cap/objpix.bin   ff4_snes->ppu->objPixelBuffer (ff4_snes->ppu->objPixelBuffer + 0x100)
dump binary memory /tmp/ff4cap/objprio.bin  ff4_snes->ppu->objPriorityBuffer (ff4_snes->ppu->objPriorityBuffer + 0x100)
dump binary memory /tmp/ff4cap/sram.bin     ff4_snes->cart->ram (ff4_snes->cart->ram + ff4_snes->cart->ramSize)

# ---- scalars: emit C assignments consumed verbatim by state_inject.c ----
printf "S->palTiming = %d;\n", ff4_snes->palTiming
printf "S->hIrqEnabled = %d;\n", ff4_snes->hIrqEnabled
printf "S->vIrqEnabled = %d;\n", ff4_snes->vIrqEnabled
printf "S->nmiEnabled = %d;\n", ff4_snes->nmiEnabled
printf "S->inNmi = %d;\n", ff4_snes->inNmi
printf "S->irqCondition = %d;\n", ff4_snes->irqCondition
printf "S->inIrq = %d;\n", ff4_snes->inIrq
printf "S->inVblank = %d;\n", ff4_snes->inVblank
printf "S->autoJoyRead = %d;\n", ff4_snes->autoJoyRead
printf "S->ppuLatch = %d;\n", ff4_snes->ppuLatch
printf "S->fastMem = %d;\n", ff4_snes->fastMem
printf "S->multiplyA = %u;\n", ff4_snes->multiplyA
printf "S->openBus = %u;\n", ff4_snes->openBus
printf "S->hPos = %u;\n", ff4_snes->hPos
printf "S->vPos = %u;\n", ff4_snes->vPos
printf "S->hTimer = %u;\n", ff4_snes->hTimer
printf "S->vTimer = %u;\n", ff4_snes->vTimer
printf "S->portAutoRead[0] = %u;\n", ff4_snes->portAutoRead[0]
printf "S->portAutoRead[1] = %u;\n", ff4_snes->portAutoRead[1]
printf "S->portAutoRead[2] = %u;\n", ff4_snes->portAutoRead[2]
printf "S->portAutoRead[3] = %u;\n", ff4_snes->portAutoRead[3]
printf "S->autoJoyTimer = %u;\n", ff4_snes->autoJoyTimer
printf "S->multiplyResult = %u;\n", ff4_snes->multiplyResult
printf "S->divideA = %u;\n", ff4_snes->divideA
printf "S->divideResult = %u;\n", ff4_snes->divideResult
printf "S->ramAdr = %u;\n", ff4_snes->ramAdr
printf "S->frames = %u;\n", ff4_snes->frames
printf "S->cycles = %llu;\n", ff4_snes->cycles
printf "S->syncCycle = %llu;\n", ff4_snes->syncCycle
printf "S->apuCatchupCycles = %.17g;\n", ff4_snes->apuCatchupCycles

printf "C->c = %d;\n", ff4_snes->cpu->c
printf "C->z = %d;\n", ff4_snes->cpu->z
printf "C->v = %d;\n", ff4_snes->cpu->v
printf "C->n = %d;\n", ff4_snes->cpu->n
printf "C->i = %d;\n", ff4_snes->cpu->i
printf "C->d = %d;\n", ff4_snes->cpu->d
printf "C->xf = %d;\n", ff4_snes->cpu->xf
printf "C->mf = %d;\n", ff4_snes->cpu->mf
printf "C->e = %d;\n", ff4_snes->cpu->e
printf "C->waiting = %d;\n", ff4_snes->cpu->waiting
printf "C->stopped = %d;\n", ff4_snes->cpu->stopped
printf "C->irqWanted = %d;\n", ff4_snes->cpu->irqWanted
printf "C->nmiWanted = %d;\n", ff4_snes->cpu->nmiWanted
printf "C->intWanted = %d;\n", ff4_snes->cpu->intWanted
printf "C->resetWanted = %d;\n", ff4_snes->cpu->resetWanted
printf "C->k = %u;\n", ff4_snes->cpu->k
printf "C->db = %u;\n", ff4_snes->cpu->db
printf "C->a = %u;\n", ff4_snes->cpu->a
printf "C->x = %u;\n", ff4_snes->cpu->x
printf "C->y = %u;\n", ff4_snes->cpu->y
printf "C->sp = %u;\n", ff4_snes->cpu->sp
printf "C->pc = %u;\n", ff4_snes->cpu->pc
printf "C->dp = %u;\n", ff4_snes->cpu->dp

printf "D->hdmaInitRequested = %d;\n", ff4_snes->dma->hdmaInitRequested
printf "D->hdmaRunRequested = %d;\n", ff4_snes->dma->hdmaRunRequested
printf "D->dmaState = %u;\n", ff4_snes->dma->dmaState
set $i = 0
while $i < 8
  printf "D->channel[%d].dmaActive = %d;\n", $i, ff4_snes->dma->channel[$i].dmaActive
  printf "D->channel[%d].hdmaActive = %d;\n", $i, ff4_snes->dma->channel[$i].hdmaActive
  printf "D->channel[%d].fixed = %d;\n", $i, ff4_snes->dma->channel[$i].fixed
  printf "D->channel[%d].decrement = %d;\n", $i, ff4_snes->dma->channel[$i].decrement
  printf "D->channel[%d].indirect = %d;\n", $i, ff4_snes->dma->channel[$i].indirect
  printf "D->channel[%d].fromB = %d;\n", $i, ff4_snes->dma->channel[$i].fromB
  printf "D->channel[%d].unusedBit = %d;\n", $i, ff4_snes->dma->channel[$i].unusedBit
  printf "D->channel[%d].doTransfer = %d;\n", $i, ff4_snes->dma->channel[$i].doTransfer
  printf "D->channel[%d].terminated = %d;\n", $i, ff4_snes->dma->channel[$i].terminated
  printf "D->channel[%d].bAdr = %u;\n", $i, ff4_snes->dma->channel[$i].bAdr
  printf "D->channel[%d].aBank = %u;\n", $i, ff4_snes->dma->channel[$i].aBank
  printf "D->channel[%d].indBank = %u;\n", $i, ff4_snes->dma->channel[$i].indBank
  printf "D->channel[%d].repCount = %u;\n", $i, ff4_snes->dma->channel[$i].repCount
  printf "D->channel[%d].unusedByte = %u;\n", $i, ff4_snes->dma->channel[$i].unusedByte
  printf "D->channel[%d].mode = %u;\n", $i, ff4_snes->dma->channel[$i].mode
  printf "D->channel[%d].aAdr = %u;\n", $i, ff4_snes->dma->channel[$i].aAdr
  printf "D->channel[%d].size = %u;\n", $i, ff4_snes->dma->channel[$i].size
  printf "D->channel[%d].tableAdr = %u;\n", $i, ff4_snes->dma->channel[$i].tableAdr
  set $i = $i + 1
end

printf "P->vramIncrementOnHigh = %d;\n", ff4_snes->ppu->vramIncrementOnHigh
printf "P->cgramSecondWrite = %d;\n", ff4_snes->ppu->cgramSecondWrite
printf "P->oamInHigh = %d;\n", ff4_snes->ppu->oamInHigh
printf "P->oamInHighWritten = %d;\n", ff4_snes->ppu->oamInHighWritten
printf "P->oamSecondWrite = %d;\n", ff4_snes->ppu->oamSecondWrite
printf "P->objPriority = %d;\n", ff4_snes->ppu->objPriority
printf "P->timeOver = %d;\n", ff4_snes->ppu->timeOver
printf "P->rangeOver = %d;\n", ff4_snes->ppu->rangeOver
printf "P->objInterlace = %d;\n", ff4_snes->ppu->objInterlace
printf "P->m7largeField = %d;\n", ff4_snes->ppu->m7largeField
printf "P->m7charFill = %d;\n", ff4_snes->ppu->m7charFill
printf "P->m7xFlip = %d;\n", ff4_snes->ppu->m7xFlip
printf "P->m7yFlip = %d;\n", ff4_snes->ppu->m7yFlip
printf "P->m7extBg = %d;\n", ff4_snes->ppu->m7extBg
printf "P->addSubscreen = %d;\n", ff4_snes->ppu->addSubscreen
printf "P->subtractColor = %d;\n", ff4_snes->ppu->subtractColor
printf "P->halfColor = %d;\n", ff4_snes->ppu->halfColor
printf "P->mathEnabled[0] = %d;\n", ff4_snes->ppu->mathEnabled[0]
printf "P->mathEnabled[1] = %d;\n", ff4_snes->ppu->mathEnabled[1]
printf "P->mathEnabled[2] = %d;\n", ff4_snes->ppu->mathEnabled[2]
printf "P->mathEnabled[3] = %d;\n", ff4_snes->ppu->mathEnabled[3]
printf "P->mathEnabled[4] = %d;\n", ff4_snes->ppu->mathEnabled[4]
printf "P->mathEnabled[5] = %d;\n", ff4_snes->ppu->mathEnabled[5]
printf "P->forcedBlank = %d;\n", ff4_snes->ppu->forcedBlank
printf "P->bg3priority = %d;\n", ff4_snes->ppu->bg3priority
printf "P->evenFrame = %d;\n", ff4_snes->ppu->evenFrame
printf "P->pseudoHires = %d;\n", ff4_snes->ppu->pseudoHires
printf "P->overscan = %d;\n", ff4_snes->ppu->overscan
printf "P->frameOverscan = %d;\n", ff4_snes->ppu->frameOverscan
printf "P->interlace = %d;\n", ff4_snes->ppu->interlace
printf "P->frameInterlace = %d;\n", ff4_snes->ppu->frameInterlace
printf "P->directColor = %d;\n", ff4_snes->ppu->directColor
printf "P->hCountSecond = %d;\n", ff4_snes->ppu->hCountSecond
printf "P->vCountSecond = %d;\n", ff4_snes->ppu->vCountSecond
printf "P->countersLatched = %d;\n", ff4_snes->ppu->countersLatched
printf "P->vramRemapMode = %u;\n", ff4_snes->ppu->vramRemapMode
printf "P->cgramPointer = %u;\n", ff4_snes->ppu->cgramPointer
printf "P->cgramBuffer = %u;\n", ff4_snes->ppu->cgramBuffer
printf "P->oamAdr = %u;\n", ff4_snes->ppu->oamAdr
printf "P->oamAdrWritten = %u;\n", ff4_snes->ppu->oamAdrWritten
printf "P->oamBuffer = %u;\n", ff4_snes->ppu->oamBuffer
printf "P->objSize = %u;\n", ff4_snes->ppu->objSize
printf "P->scrollPrev = %u;\n", ff4_snes->ppu->scrollPrev
printf "P->scrollPrev2 = %u;\n", ff4_snes->ppu->scrollPrev2
printf "P->mosaicSize = %u;\n", ff4_snes->ppu->mosaicSize
printf "P->mosaicStartLine = %u;\n", ff4_snes->ppu->mosaicStartLine
printf "P->m7prev = %u;\n", ff4_snes->ppu->m7prev
printf "P->window1left = %u;\n", ff4_snes->ppu->window1left
printf "P->window1right = %u;\n", ff4_snes->ppu->window1right
printf "P->window2left = %u;\n", ff4_snes->ppu->window2left
printf "P->window2right = %u;\n", ff4_snes->ppu->window2right
printf "P->clipMode = %u;\n", ff4_snes->ppu->clipMode
printf "P->preventMathMode = %u;\n", ff4_snes->ppu->preventMathMode
printf "P->fixedColorR = %u;\n", ff4_snes->ppu->fixedColorR
printf "P->fixedColorG = %u;\n", ff4_snes->ppu->fixedColorG
printf "P->fixedColorB = %u;\n", ff4_snes->ppu->fixedColorB
printf "P->brightness = %u;\n", ff4_snes->ppu->brightness
printf "P->mode = %u;\n", ff4_snes->ppu->mode
printf "P->ppu1openBus = %u;\n", ff4_snes->ppu->ppu1openBus
printf "P->ppu2openBus = %u;\n", ff4_snes->ppu->ppu2openBus
printf "P->vramPointer = %u;\n", ff4_snes->ppu->vramPointer
printf "P->vramIncrement = %u;\n", ff4_snes->ppu->vramIncrement
printf "P->vramReadBuffer = %u;\n", ff4_snes->ppu->vramReadBuffer
printf "P->objTileAdr1 = %u;\n", ff4_snes->ppu->objTileAdr1
printf "P->objTileAdr2 = %u;\n", ff4_snes->ppu->objTileAdr2
printf "P->hCount = %u;\n", ff4_snes->ppu->hCount
printf "P->vCount = %u;\n", ff4_snes->ppu->vCount
printf "P->m7matrix[0] = %d;\n", ff4_snes->ppu->m7matrix[0]
printf "P->m7matrix[1] = %d;\n", ff4_snes->ppu->m7matrix[1]
printf "P->m7matrix[2] = %d;\n", ff4_snes->ppu->m7matrix[2]
printf "P->m7matrix[3] = %d;\n", ff4_snes->ppu->m7matrix[3]
printf "P->m7matrix[4] = %d;\n", ff4_snes->ppu->m7matrix[4]
printf "P->m7matrix[5] = %d;\n", ff4_snes->ppu->m7matrix[5]
printf "P->m7matrix[6] = %d;\n", ff4_snes->ppu->m7matrix[6]
printf "P->m7matrix[7] = %d;\n", ff4_snes->ppu->m7matrix[7]
printf "P->m7startX = %d;\n", ff4_snes->ppu->m7startX
printf "P->m7startY = %d;\n", ff4_snes->ppu->m7startY
set $b = 0
while $b < 4
  printf "P->bgLayer[%d].tilemapWider = %d;\n", $b, ff4_snes->ppu->bgLayer[$b].tilemapWider
  printf "P->bgLayer[%d].tilemapHigher = %d;\n", $b, ff4_snes->ppu->bgLayer[$b].tilemapHigher
  printf "P->bgLayer[%d].bigTiles = %d;\n", $b, ff4_snes->ppu->bgLayer[$b].bigTiles
  printf "P->bgLayer[%d].mosaicEnabled = %d;\n", $b, ff4_snes->ppu->bgLayer[$b].mosaicEnabled
  printf "P->bgLayer[%d].hScroll = %u;\n", $b, ff4_snes->ppu->bgLayer[$b].hScroll
  printf "P->bgLayer[%d].vScroll = %u;\n", $b, ff4_snes->ppu->bgLayer[$b].vScroll
  printf "P->bgLayer[%d].tilemapAdr = %u;\n", $b, ff4_snes->ppu->bgLayer[$b].tilemapAdr
  printf "P->bgLayer[%d].tileAdr = %u;\n", $b, ff4_snes->ppu->bgLayer[$b].tileAdr
  set $b = $b + 1
end
set $l = 0
while $l < 5
  printf "P->layer[%d].mainScreenEnabled = %d;\n", $l, ff4_snes->ppu->layer[$l].mainScreenEnabled
  printf "P->layer[%d].subScreenEnabled = %d;\n", $l, ff4_snes->ppu->layer[$l].subScreenEnabled
  printf "P->layer[%d].mainScreenWindowed = %d;\n", $l, ff4_snes->ppu->layer[$l].mainScreenWindowed
  printf "P->layer[%d].subScreenWindowed = %d;\n", $l, ff4_snes->ppu->layer[$l].subScreenWindowed
  set $l = $l + 1
end
set $w = 0
while $w < 6
  printf "P->windowLayer[%d].window1enabled = %d;\n", $w, ff4_snes->ppu->windowLayer[$w].window1enabled
  printf "P->windowLayer[%d].window1inversed = %d;\n", $w, ff4_snes->ppu->windowLayer[$w].window1inversed
  printf "P->windowLayer[%d].window2enabled = %d;\n", $w, ff4_snes->ppu->windowLayer[$w].window2enabled
  printf "P->windowLayer[%d].window2inversed = %d;\n", $w, ff4_snes->ppu->windowLayer[$w].window2inversed
  printf "P->windowLayer[%d].maskLogic = %u;\n", $w, ff4_snes->ppu->windowLayer[$w].maskLogic
  set $w = $w + 1
end

printf "I1->type = %u;\n", ff4_snes->input1->type
printf "I1->latchLine = %d;\n", ff4_snes->input1->latchLine
printf "I1->currentState = %u;\n", ff4_snes->input1->currentState
printf "I1->latchedState = %u;\n", ff4_snes->input1->latchedState
printf "I2->type = %u;\n", ff4_snes->input2->type
printf "I2->latchLine = %d;\n", ff4_snes->input2->latchLine
printf "I2->currentState = %u;\n", ff4_snes->input2->currentState
printf "I2->latchedState = %u;\n", ff4_snes->input2->latchedState

printf "S->apu->inPorts[0] = %u;\n", ff4_snes->apu->inPorts[0]
printf "S->apu->inPorts[1] = %u;\n", ff4_snes->apu->inPorts[1]
printf "S->apu->inPorts[2] = %u;\n", ff4_snes->apu->inPorts[2]
printf "S->apu->inPorts[3] = %u;\n", ff4_snes->apu->inPorts[3]
printf "S->apu->outPorts[0] = %u;\n", ff4_snes->apu->outPorts[0]
printf "S->apu->outPorts[1] = %u;\n", ff4_snes->apu->outPorts[1]
printf "S->apu->outPorts[2] = %u;\n", ff4_snes->apu->outPorts[2]
printf "S->apu->outPorts[3] = %u;\n", ff4_snes->apu->outPorts[3]

monitor resume
disconnect
quit
