export function stutter_pad_setup(audioEngine){
    const pad = document.querySelector(".stutter_pad");
    const stages = document.querySelectorAll(".stutter_stage");
    const cursor = document.querySelector(".stutter_cursor");

    let dragging = false;

    pad.addEventListener("mousedown", (event) => {
        dragging = true;
        cursor.style.display = "block";
        updateStutterPosition(event);
    });

    document.addEventListener("mousemove", (event) => {
        if (dragging){
            updateStutterPosition(event);
        }
    });

    document.addEventListener("mouseup", (event) => {
        if (dragging){
            dragging = false;
            cursor.style.display = "none";
            audioEngine.updateStutter(-1);
        }
    });

    function updateStutterPosition(event){
        const pad_size = pad.getBoundingClientRect();

        let x = event.clientX - pad_size.left;

        x = Math.max(0, Math.min(x, pad_size.width));

        cursor.style.left = `${x}px`;

        const stage = Math.min(stages.length - 1, Math.floor((x / pad_size.width) * stages.length));

        audioEngine.updateStutter(stage);
    }
}