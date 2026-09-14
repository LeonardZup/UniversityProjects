export function filter_pad_setup(audioEngine){
    const pad = document.querySelector(".filter_pad");
    const cursor = document.querySelector(".filter_cursor");

    let dragging = false;

    pad.addEventListener("mousedown", (event) => {
        dragging = true;
        cursor.style.display = "block";
        updateFilterPosition(event);
    });

    document.addEventListener("mousemove", (event) => {
        if (dragging){
            updateFilterPosition(event);
        }
    });

    document.addEventListener("mouseup", (event) => {
        if (dragging){
            dragging = false;
            cursor.style.display = "none";
            audioEngine.updateFilter(0.5, 0);
        }
    });

    function updateFilterPosition(event){
        const pad_size = pad.getBoundingClientRect();

        let x = event.clientX - pad_size.left;
        let y = event.clientY - pad_size.top;

        x = Math.max(0, Math.min(x, pad_size.width));
        y = Math.max(0, Math.min(y, pad_size.height));

        const cut_value = x / pad_size.width;
        const res_value = 1 - (y / pad_size.height);

        cursor.style.left = `${x}px`;
        cursor.style.top = `${y}px`;

        audioEngine.updateFilter(cut_value, res_value);
    }
}