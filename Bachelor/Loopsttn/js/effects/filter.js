export class Filter{
    constructor(context, tempo){
        this.context = context;
        this.tempo = tempo;

        this.lpfilterNode = this.context.createBiquadFilter();
        this.lpfilterNode.type = "lowpass";
        this.lpfilterNode.frequency.value = 20000; //cutoff
        this.lpfilterNode.Q.value = 0.707; //resonance

        this.hpfilterNode = this.context.createBiquadFilter();
        this.hpfilterNode.type = "highpass";
        this.hpfilterNode.frequency.value = 20;
        this.hpfilterNode.Q.value = 0.707;

        this.fMin = 20;
        this.fMax = 20000;
        this.qMin = 0.707;
        this.qMax = 25;
        this.smoothingTime = 0.005;

        this.lpfilterNode.connect(this.hpfilterNode);

        this.input = this.lpfilterNode;
        this.output = this.hpfilterNode;
    }

    updateFilter(cut_value, res_value){
        if (cut_value <= 0.5) { 
            const cut_freq = cut_value / 0.5;
            const freq = this.fMin * Math.pow((this.fMax / this.fMin), cut_freq); //conversione da scala lineare a logaritmica per le ottave
            this.lpfilterNode.frequency.setTargetAtTime(freq, this.context.currentTime, this.smoothingTime);

            this.hpfilterNode.frequency.setTargetAtTime(this.fMin, this.context.currentTime, this.smoothingTime);
        } else{
            const cut_freq = (cut_value - 0.5) / 0.5;
            const freq = this.fMin * Math.pow((this.fMax / this.fMin), cut_freq);
            this.hpfilterNode.frequency.setTargetAtTime(freq, this.context.currentTime, this.smoothingTime);

            this.lpfilterNode.frequency.setTargetAtTime(this.fMax, this.context.currentTime, this.smoothingTime);
        }
        const q = this.qMin + (this.qMax - this.qMin) * Math.pow(res_value, 2);
        this.lpfilterNode.Q.setTargetAtTime(q, this.context.currentTime, this.smoothingTime);
        this.hpfilterNode.Q.setTargetAtTime(q, this.context.currentTime, this.smoothingTime);
    }
}