export class Delay{
    constructor(context, tempo){
        this.context = context;

        this.dryNode = this.context.createGain();
        this.delayNode = this.context.createDelay(4); //max 4 secondi di delay
        this.feedbackNode = this.context.createGain();
        this.wetNode = this.context.createGain();

        this.dryNode.connect(this.delayNode);
        this.dryNode.connect(this.wetNode);
        this.delayNode.connect(this.feedbackNode);
        this.feedbackNode.connect(this.delayNode);
        this.feedbackNode.connect(this.wetNode);

        this.delayNode.delayTime.value = 0;
        this.feedbackNode.gain.value = 0;

        this.maxPeriod = 8;
        this.maxFeedback = 0.5;
        this.smoothingTime = 0.01;

        this.input = this.dryNode;
        this.output = this.wetNode;
    }

    updateDelay(time_value, feedback_value, tempo){
        if (time_value == 0 && feedback_value == 0){
            this.delayNode.delayTime.setTargetAtTime(0, this.context.currentTime, this.smoothingTime);
            this.feedbackNode.gain.setTargetAtTime(0, this.context.currentTime, this.smoothingTime);
        } else{
            const beatDuration = 60 / tempo;
            const divisions = Math.max(1, Math.ceil(this.maxPeriod * time_value));
            const delayTime = beatDuration / divisions;
            this.delayNode.delayTime.setTargetAtTime(delayTime, this.context.currentTime, this.smoothingTime);

            const feedback = this.maxFeedback * feedback_value;
            this.feedbackNode.gain.setTargetAtTime(feedback, this.context.currentTime, this.smoothingTime);
        }
    }
}