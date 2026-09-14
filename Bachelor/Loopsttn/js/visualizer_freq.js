export function visualizer_freq_setup(audioEngine) {
    const canvas = document.querySelector(".freq_canvas");
    const ctx = canvas.getContext("2d");

    canvas.width = canvas.clientWidth;
    canvas.height = canvas.clientHeight;

    function updateVisualizer() {
        requestAnimationFrame(updateVisualizer);

        audioEngine.visualizer.updateFreqLevels();

        drawFreqBars(ctx, audioEngine.visualizer.freqData);
    }

    function drawFreqBars(ctx, data) {
        const width = ctx.canvas.width;
        const height = ctx.canvas.height;
        ctx.clearRect(0, 0, width, height);

        const freqLen = Math.floor(data.length * 0.6);
        const barWidth = width / freqLen;

        const minDb = -100;
        const maxDb = -10;

        for (let i = 0; i < freqLen; i++) {
            let db = data[i];
            db = Math.max(minDb, Math.min(maxDb, db));

            const percent = (db - minDb) / (maxDb - minDb);

            const barHeight = percent * height;
            const x = i * barWidth;
            const y = height - barHeight;

            ctx.fillStyle = "white";
            ctx.fillRect(x, y, barWidth - 2, barHeight);
        }
    }   

    updateVisualizer();
}