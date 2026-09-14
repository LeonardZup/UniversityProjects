export function delay_pad_setup(audioEngine){
    const pad = document.querySelector(".delay_pad");
    const cursor = document.querySelector(".delay_cursor");

    let dragging = false;

    pad.addEventListener("mousedown", (event) => {
        dragging = true;
        cursor.style.display = "block";
        updateDelayPosition(event);
    });

    document.addEventListener("mousemove", (event) => {
        if (dragging){
            updateDelayPosition(event);
        }
    });

    document.addEventListener("mouseup", (event) => {
        if (dragging){
            dragging = false;
            cursor.style.display = "none";
            audioEngine.updateDelay(0, 0);
        }
    });

    function updateDelayPosition(event){
        const pad_size = pad.getBoundingClientRect();

        let x = event.clientX - pad_size.left;
        let y = event.clientY - pad_size.top;

        x = Math.max(0, Math.min(x, pad_size.width));
        y = Math.max(0, Math.min(y, pad_size.height));

        const time_value = x / pad_size.width;
        const feedback_value = 1 - (y / pad_size.height);

        cursor.style.left = `${x}px`;
        cursor.style.top = `${y}px`;

        audioEngine.updateDelay(time_value, feedback_value);
    }
}