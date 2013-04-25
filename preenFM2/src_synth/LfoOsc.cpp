/*
 * Copyright 2011 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier.hosxe@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LfoOsc.h"


extern float noise[32];

void LfoOsc::init(struct LfoParams *lfoParams, Matrix *matrix, SourceEnum source, DestinationEnum dest) {
    Lfo::init(matrix, source, dest);
    this->type = LFO_SAW;
    this->ramp = 0;
    this->currentRamp = 0;
    this->lfo = lfoParams;
    this->currentRamp =  0;
    valueChanged(3);
    this->destination = dest;
    this->currentRandomValue = 0.0f;

    ticks = 1536;
    midiClock(0, true);
}


void LfoOsc::midiClock(int songPosition, bool computeStep) {
    switch ((int)(lfo->freq * 10.0f)) {
    case LFO_MIDICLOCK_MC_DIV_16:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * .0625f;
                ticks = 0;
            }
            phase = (songPosition & 0x3E) * 0.015625f;
        }
        break;
    case LFO_MIDICLOCK_MC_DIV_8:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * .125f;
                ticks = 0;
            }
            phase = (songPosition & 0x1E) * 0.03125f ;
        }
        break;
    case LFO_MIDICLOCK_MC_DIV_4:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * .25f;
                ticks = 0;
            }
            phase = (songPosition & 0xE) * 0.0625f;
        }
        break;
    case LFO_MIDICLOCK_MC_DIV_2:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * .5f;
                ticks = 0;
            }
            // 0,2,4,6
            phase = (songPosition & 0x6) * .125f;
        }
        break;
    case LFO_MIDICLOCK_MC:
        // Midi Clock
        if ((songPosition & 0x1) == 0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY;
                ticks = 0;
            }
            // 0 or 2 -> 0 ou .5
            phase = (songPosition & 0x2) * .25f;
        }
        break;
    case LFO_MIDICLOCK_MC_TIME_2:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * 2.0f;
                ticks = 0;
            }
            phase = 0;
        }
        break;
    case LFO_MIDICLOCK_MC_TIME_3:
        if ((songPosition & 0x3)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY;
                ticks = 0;
            }
            phase = 0;
        }
        break;
    case LFO_MIDICLOCK_MC_TIME_4:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * 4.0f;
                ticks = 0;
            }
            phase = 0;
        }
        break;
    case LFO_MIDICLOCK_MC_TIME_8:
        if ((songPosition & 0x1)==0) {
            if (computeStep) {
                currentFreq = ticks * BLOCK_SIZE / PREENFM_FREQUENCY * 8.0f;
                ticks = 0;
            }
            phase = 0;
        }
        break;
    }
}


void LfoOsc::nextValueInMatrix() {
    float lfoValue = 0;

    ticks ++;

    if ((lfo->freq * 10.0f) < LFO_MIDICLOCK_MC_DIV_16) {
        currentFreq = lfo->freq + this->matrix->getDestination(destination);
    }
    phase += currentFreq * PREENFM_FREQUENCY_INVERSED_LFO;

    switch ((int)lfo->shape) {
    case LFO_SAW:
    {
        phase -= (phase >= 1.0f) * 1.0f;
        if (phase < .5f) {
            lfoValue = phase * 4.0f -1.0f ;
        } else {
            lfoValue = 1.0f - (phase - .5f ) * 4.0f;
        }
        break;
    }
    case LFO_RAMP:
        phase -= (phase >= 1.0f) * 1.0f;
        lfoValue = -1.0f + phase * 2.0f;
        break;
    case LFO_SIN:
        phase -= (phase >= 1.0f) * 1.0f;
        lfoValue = sinTable[ (int) (phase * waveTables[0].max) ];
        break;
    case LFO_SQUARE:
        phase -= (phase >= 1.0f) * 1.0f;
        if (phase < .5) {
            lfoValue = -1.0f;
        } else {
            lfoValue = 1.0f;
        }
        break;
    case LFO_RANDOM:
        if (phase >= 1.0f) {
            phase -= 1.0f;
            currentRandomValue = noise[0];
        }

        lfoValue = currentRandomValue;
        break;
    }


    if (currentRamp < ramp) {
        lfoValue = lfoValue * currentRamp  * rampInv ;
        currentRamp += PREENFM_FREQUENCY_INVERSED_LFO;
    }

    lfoValue += lfo->bias;

    matrix->setSource(source, lfoValue);
}


