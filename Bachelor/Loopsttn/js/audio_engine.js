import {Filter} from "./effects/filter.js";
import {Delay} from "./effects/delay.js";
import {Stutter} from "./effects/stutter.js";
import {Roll} from "./effects/roll.js";
import {Gain} from "./effects/gain.js";
import {Visualizer} from "./effects/visualizer.js";

const LOOP_STATE = {
    STOPPED: "STOPPED",
    PLAYING: "PLAYING",
    WAIT_PLAYING: "WAIT_PLAYING",
    WAIT_STOPPED: "WAIT_STOPPED"
}

export class AudioEngine{
    constructor(){
        this.context = new AudioContext();

        this.gridState = new Array(30).fill(LOOP_STATE.STOPPED);
        this.waitTimes = new Array(30).fill(-1); //tempi di attesa per stop e play
        this.pendingStops = new Array(30).fill(null);
        this.loopBuffers = [];
        this.activeLoops = [];

        this.tempo = 0;
        this.startTime = 0;
        this.paused = true;

        this.config = null;

        //traccia con tutti i loop in riproduzione
        this.masterTrack = this.context.createGain();
        this.masterTrack.gain.value = 1;

        //effetti
        this.filter = new Filter(this.context, this.tempo);
        this.delay = new Delay(this.context, this.tempo);
        this.stutter = new Stutter(this.context, this.tempo);
        this.roll = new Roll(this.context, this.tempo);
        this.gain = new Gain(this.context, this.tempo);
        this.visualizer = new Visualizer(this.context, this.tempo);

        //catena di effetti
        this.masterTrack.connect(this.filter.input);
        this.filter.output.connect(this.delay.input);
        this.delay.output.connect(this.stutter.input);
        this.stutter.output.connect(this.roll.input);
        this.roll.output.connect(this.gain.input);
        this.gain.output.connect(this.visualizer.input);
        this.visualizer.output.connect(this.context.destination);
    }

    async loadSet(set){
        this.config = set;

        this.pauseAll();
        this.gridState = new Array(30).fill(LOOP_STATE.STOPPED);
        this.loopBuffers = [];
        this.activeLoops = [];
        this.startTime = 0;

        this.paused = true;

        this.tempo = this.config.tempo;

        for (let i = 0; i < 30; i++){
            const response = await fetch(`/loop_sets/${this.config.folder}/${this.config.loops[i].file}`);
            const audioBuffer = await response.arrayBuffer();
            this.loopBuffers[i] = await this.context.decodeAudioData(audioBuffer);
        }
    }

    getLoopTime(loop_id){
        const loopBeats = this.config.loops[loop_id].beats;
        const beatDuration = 60 / this.tempo;
        const loopDuration = beatDuration * loopBeats;

        const timePassed = this.context.currentTime - this.startTime;
        const nextBar = Math.ceil(timePassed / loopDuration);
        return this.startTime + (nextBar * loopDuration);
    }

    toggleLoop(loop_id){
        if (this.paused) {
            if (this.gridState[loop_id] == LOOP_STATE.STOPPED) {
                this.gridState[loop_id] = LOOP_STATE.WAIT_PLAYING;
            } else {
                this.gridState[loop_id] = LOOP_STATE.STOPPED;
            }
            return;
        }

        switch (this.gridState[loop_id]){
            case LOOP_STATE.STOPPED:
                this.gridState[loop_id] = LOOP_STATE.WAIT_PLAYING;
                this.waitTimes[loop_id] = this.getLoopTime(loop_id);
                this.startLoop(loop_id, this.waitTimes[loop_id]);
                break;
            case LOOP_STATE.WAIT_PLAYING:
                this.gridState[loop_id] = LOOP_STATE.STOPPED;
                this.waitTimes[loop_id] = -1;
                this.stopLoop(loop_id, this.context.currentTime);
                this.activeLoops[loop_id] = null;
                break;
            case LOOP_STATE.PLAYING:
                this.gridState[loop_id] = LOOP_STATE.WAIT_STOPPED;
                this.waitTimes[loop_id] = this.getLoopTime(loop_id);
                this.stopLoop(loop_id, this.waitTimes[loop_id]);
                break;
            case LOOP_STATE.WAIT_STOPPED:
                this.pendingStops[loop_id] = this.activeLoops[loop_id];
                this.activeLoops[loop_id] = null;
                this.gridState[loop_id] = LOOP_STATE.WAIT_PLAYING;
                this.waitTimes[loop_id] = this.getLoopTime(loop_id);
                this.startLoop(loop_id, this.waitTimes[loop_id]);
                break;
        }
    }

    updateStates() {
        if (this.paused){
            return;
        }
        for (let i = 0; i < this.gridState.length; i++) {
            if (this.waitTimes[i] != -1 && this.context.currentTime >= this.waitTimes[i]) {
                if (this.gridState[i] == LOOP_STATE.WAIT_PLAYING) {
                    this.gridState[i] = LOOP_STATE.PLAYING;
                } else if (this.gridState[i] == LOOP_STATE.WAIT_STOPPED) {
                    this.gridState[i] = LOOP_STATE.STOPPED;
                    this.activeLoops[i] = null;
                }
                this.waitTimes[i] = -1;
            }
        }
    }

    startLoop(loop_id, startTime){
        if (!this.activeLoops[loop_id]){
            const source = this.context.createBufferSource();
            source.buffer = this.loopBuffers[loop_id];
            source.connect(this.masterTrack);
            source.start(startTime);
            source.loop = true;

            this.activeLoops[loop_id] = source;
        }
    }

    stopLoop(loop_id, stopTime){
        if (this.activeLoops[loop_id]){
            this.activeLoops[loop_id].stop(stopTime);
        }
    }

    pauseAll(){
        for (let i = 0; i < this.gridState.length; i++){
            if (this.activeLoops[i]){
                this.activeLoops[i].stop();
                this.activeLoops[i] = null;
            }
            if (this.pendingStops[i]) {
                this.pendingStops[i].stop();
                this.pendingStops[i] = null;
            }

            this.waitTimes[i] = -1;

            if (this.gridState[i] == LOOP_STATE.WAIT_STOPPED) {
                this.gridState[i] = LOOP_STATE.STOPPED;
            }
        }
        this.paused = true;
    }

    resumeAll(){
        this.startTime = this.context.currentTime;
        
        for (let i = 0; i < this.gridState.length; i++){
            if (this.gridState[i] == LOOP_STATE.PLAYING || this.gridState[i] == LOOP_STATE.WAIT_PLAYING){
                this.startLoop(i, this.startTime);
                this.gridState[i] = LOOP_STATE.PLAYING;
                this.waitTimes[i] = -1;
            }
        }
        this.paused = false;
    }

    updateFilter(cut_value, res_value){
        this.filter.updateFilter(cut_value, res_value);
    }

    updateDelay(time_value, feedback_value){
        this.delay.updateDelay(time_value, feedback_value, this.tempo);
    }

    updateStutter(stutter_value){
        this.stutter.updateStutter(stutter_value, this.startTime, this.tempo);
    }

    updateRoll(roll_value){
        const activeBuffers = [];
        for (let i = 0; i < this.gridState.length; i++){
            if (this.gridState[i] == LOOP_STATE.PLAYING){
                activeBuffers.push(this.loopBuffers[i]);
            }
        }
        this.roll.updateRoll(roll_value, activeBuffers, this.startTime, this.tempo);
    }

    updateGain(gain_value){
        this.gain.updateGain(gain_value);
    }
}