export function visualizer_bar_setup(audioEngine) {
    const leftCanvas = document.querySelector(".left_visualizer_bar");
    const rightCanvas = document.querySelector(".right_visualizer_bar");

    const leftCtx = leftCanvas.getContext("2d");
    const rightCtx = rightCanvas.getContext("2d");

    leftCanvas.width = leftCanvas.clientWidth;
    leftCanvas.height = leftCanvas.clientHeight;
    rightCanvas.width = rightCanvas.clientWidth;
    rightCanvas.height = rightCanvas.clientHeight;

    function updateVisualizer() {
        requestAnimationFrame(updateVisualizer);

        audioEngine.visualizer.updateLevels();

        drawBar(leftCtx, audioEngine.visualizer.leftData);
        drawBar(rightCtx, audioEngine.visualizer.rightData);
    }

    function drawBar(ctx, data) {
        const width = ctx.canvas.width;
        const height = ctx.canvas.height;
        ctx.clearRect(0, 0, width, height);

        let db = -100;
        for (let i = 0; i < data.length; i++) {
            if (data[i] > db) {
                db = data[i];
            }
        }

        const minDb = -100;
        const maxDb = -10;

        db = Math.max(minDb, Math.min(maxDb, db));

        const percent = (db - minDb) / (maxDb - minDb);

        const barHeight = percent * height;
        const x = 0;
        const y = height - barHeight;

        ctx.fillStyle = "white";
        ctx.fillRect(x, y, width, barHeight);
    }

    updateVisualizer();
}