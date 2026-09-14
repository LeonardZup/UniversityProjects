export class Visualizer{
    constructor(context, tempo){
        this.context = context;
        this.tempo = tempo;

        //nodi
        this.freqAnalyserNode = context.createAnalyser();
        this.freqAnalyserNode.fftSize = 128;

        this.splitterNode = context.createChannelSplitter(2);

        this.leftAnalyserNode = context.createAnalyser();
        this.leftAnalyserNode.fftSize = 128;

        this.rightAnalyserNode = context.createAnalyser();
        this.rightAnalyserNode.fftSize = 128;

        this.outputNode = context.createGain();

        //catena
        this.freqAnalyserNode.connect(this.splitterNode);
        this.splitterNode.connect(this.leftAnalyserNode, 0);
        this.splitterNode.connect(this.rightAnalyserNode, 1);
        this.leftAnalyserNode.connect(this.outputNode);
        this.rightAnalyserNode.connect(this.outputNode);

        this.freqData = new Float32Array(this.freqAnalyserNode.frequencyBinCount);
        
        this.leftData = new Float32Array(this.leftAnalyserNode.frequencyBinCount);
        this.rightData = new Float32Array(this.rightAnalyserNode.frequencyBinCount);

        this.input = this.freqAnalyserNode; 
        this.output = this.outputNode;
    }

    updateLevels(){
        this.leftAnalyserNode.getFloatFrequencyData(this.leftData);
        this.rightAnalyserNode.getFloatFrequencyData(this.rightData);
    }

    updateFreqLevels(){
        this.freqAnalyserNode.getFloatFrequencyData(this.freqData);
    }
}