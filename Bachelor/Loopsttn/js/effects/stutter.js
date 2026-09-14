export class Stutter{
    constructor(context, tempo){
        this.context = context;
        this.tempo = tempo;
        this.startTime = 0;

        this.stutterNode = this.context.createGain();
        this.stutterNode.gain.value = 1;

        this.stuttering = false;
        this.stutterPeriod = 0;
        this.smoothingTime = 0.001; 
        this.scheduleTime = 0;

        this.input = this.stutterNode;
        this.output = this.stutterNode;
    }
    
    updateStutter(stutter_value, startTime, tempo){
        this.startTime = startTime;
        this.tempo = tempo;

        if (stutter_value < 0){
            this.stuttering = false;
            this.stutterNode.gain.cancelScheduledValues(this.context.currentTime);
            this.stutterNode.gain.setTargetAtTime(1, this.context.currentTime, this.smoothingTime);
        } else{
            const beatDuration = 60 / this.tempo;
            this.stutterPeriod = beatDuration / (Math.pow(2, stutter_value));
            const timePassed = this.context.currentTime - this.startTime;
            const nextPeriod = Math.floor(timePassed / this.stutterPeriod) * this.stutterPeriod;
            this.scheduleTime = this.startTime + nextPeriod;
            this.stutterNode.gain.cancelScheduledValues(this.context.currentTime);
            if (!this.stuttering){
                this.stuttering = true;
                this.nextStutter();
            }
        }
    }
    
    nextStutter(){
        if (!this.stuttering){
            return; 
        }

        const scheduleLimit = 0.1;

        while (this.context.currentTime + scheduleLimit > this.scheduleTime){
            this.stutterNode.gain.setTargetAtTime(1, this.scheduleTime, this.smoothingTime);
            this.stutterNode.gain.setTargetAtTime(0, this.scheduleTime + (this.stutterPeriod / 2), this.smoothingTime);
            this.scheduleTime += this.stutterPeriod;
        }

        setTimeout(() => this.nextStutter(), 20);
    }
}