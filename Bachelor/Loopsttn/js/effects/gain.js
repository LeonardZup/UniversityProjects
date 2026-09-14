export class Gain{
    constructor(context, tempo){
        this.context = context;
        this.tempo = tempo;

        this.gainNode = this.context.createGain();
        this.gainNode.gain.value = 0.5;

        this.maxGain = 0.8;
        this.smoothingTime = 0.01;

        this.input = this.gainNode;
        this.output = this.gainNode;
    }

    updateGain(gain_value){
        if (gain_value < 0.01){ //azzeramento del volume sotto una certa soglia
            this.gainNode.gain.setTargetAtTime(0, this.context.currentTime, this.smoothingTime);
        } else{
            const dbRange = -60;
            const db = (1 - gain_value) * dbRange;
    
            const gain = Math.pow(10, (db / 20)) * this.maxGain; // 10 ^ (db / 20) per conversione da dB a guadagno lineare 

            this.gainNode.gain.setTargetAtTime(gain, this.context.currentTime, this.smoothingTime);
        }
    }
}