export function gain_slider_setup(audioEngine){
    const slider = document.querySelector(".gain_slider");
    const cursor = document.querySelector(".gain_cursor");

    let dragging = false

    slider.addEventListener("mousedown", (event) => {
        dragging = true;
        updateGainPosition(event);
    });

    document.addEventListener("mousemove", (event) => {
        if (dragging){
            updateGainPosition(event);
        }
    })

    document.addEventListener("mouseup", (event) => {
        if (dragging){
            dragging = false;
        }
    });

    function updateGainPosition(event){
        const slider_size = slider.getBoundingClientRect();
        const cursor_size = cursor.getBoundingClientRect();

        const cursor_gap = (slider_size.width - cursor_size.width) / 2;
        const cursor_radius = cursor_size.height / 2;

        let y = event.clientY - slider_size.top;

        const min_y = cursor_gap + cursor_radius;
        const max_y = slider_size.height - cursor_gap - cursor_radius;
        
        y = Math.max(min_y, Math.min(y, max_y));

        const slider_range = max_y - min_y;

        const gain_value = 1 - ((y - min_y) / slider_range);

        cursor.style.top = `${y}px`;

        audioEngine.updateGain(gain_value);
    }
}