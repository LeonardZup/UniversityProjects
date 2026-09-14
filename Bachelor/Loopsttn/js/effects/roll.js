export class Roll {
    constructor(context, tempo) {
        this.context = context;
        this.tempo = tempo;
        this.startTime = 0;

        this.inputNode = this.context.createGain();
        this.rollNode = this.context.createGain();
        this.outputNode = this.context.createGain();
        
        this.inputNode.connect(this.outputNode);
        this.rollNode.connect(this.outputNode);

        this.activeLoops = [];
        this.rollTime = -1;
        this.rollStart = 0;
        this.smoothingTime = 0.01;

        this.input = this.inputNode;
        this.output = this.outputNode;
    }

    updateRoll(roll_value, activeBuffers, startTime, tempo) {
        this.tempo = tempo;
        this.startTime = startTime;
        const beatDuration = 60 / this.tempo;
        const newRollTime = (2 * beatDuration) / Math.pow(2, roll_value);

        if (roll_value < 0) {
            this.inputNode.gain.cancelScheduledValues(this.context.currentTime);
            this.rollNode.gain.cancelScheduledValues(this.context.currentTime);
            
            this.inputNode.gain.setTargetAtTime(1, this.context.currentTime, this.smoothingTime);
            this.rollNode.gain.setTargetAtTime(0, this.context.currentTime, this.smoothingTime);
            this.stopAudio(this.context.currentTime);
            this.rollStart = 0;
            this.rollTime = -1;
        } else {
            if (newRollTime != this.rollTime){
                if (this.rollTime < 0) {
                    this.rollStart = Math.floor((this.context.currentTime - this.startTime) / newRollTime) * newRollTime;
                }
                this.rollTime = newRollTime;

                const waitTime = this.rollTime - ((this.context.currentTime - this.startTime) % this.rollTime);

                this.inputNode.gain.cancelScheduledValues(this.context.currentTime);
                this.rollNode.gain.cancelScheduledValues(this.context.currentTime);

                this.inputNode.gain.setTargetAtTime(0, this.context.currentTime + waitTime, this.smoothingTime);
                this.rollNode.gain.setTargetAtTime(1, this.context.currentTime + waitTime, this.smoothingTime);

                this.stopAudio(this.context.currentTime + waitTime);
                this.syncAudio(activeBuffers, waitTime);
            }
        }
    }

    syncAudio(activeBuffers, waitTime) {
        const beatDuration = 60 / this.tempo;
        const barDuration = beatDuration * 4 * 2;
        
        for (let i = 0; i < activeBuffers.length; i++) {
            const buffer = activeBuffers[i];
            const source = this.context.createBufferSource();
            source.buffer = buffer;
            source.connect(this.rollNode);
            source.loop = true;

            let startRep = Math.max(0, Math.min(this.rollStart % barDuration, buffer.duration - this.rollTime));

            source.loopStart = startRep;
            source.loopEnd = startRep + this.rollTime;

            source.start(this.context.currentTime + waitTime, startRep); 
            this.activeLoops.push(source);
        }
    }
    
    stopAudio(stopTime) {
        while (this.activeLoops.length > 0) {
            const source = this.activeLoops.pop();
            if (source) {
                source.stop(stopTime);
            }
        }
    }
}