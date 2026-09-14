export function roll_pad_setup(audioEngine){
    const pad = document.querySelector(".roll_pad");
    const stages = document.querySelectorAll(".roll_stage");
    const cursor = document.querySelector(".roll_cursor");

    let dragging = false;

    pad.addEventListener("mousedown", (event) => {
        dragging = true;
        cursor.style.display = "block";
        updateRollPosition(event);
    });

    document.addEventListener("mousemove", (event) => {
        if (dragging){
            updateRollPosition(event);
        }
    });

    document.addEventListener("mouseup", (event) => {
        if (dragging){
            dragging = false;
            cursor.style.display = "none";
            audioEngine.updateRoll(-1);
        }
    });

    function updateRollPosition(event){
        const pad_size = pad.getBoundingClientRect();

        let x = event.clientX - pad_size.left;

        x = Math.max(0, Math.min(x, pad_size.width));

        cursor.style.left = `${x}px`;

        const stage = Math.min(stages.length - 1, Math.floor((x / pad_size.width) * stages.length));

        audioEngine.updateRoll(stage);
    }
}