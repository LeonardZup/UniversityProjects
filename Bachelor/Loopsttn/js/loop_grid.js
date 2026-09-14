export function loop_grid_setup(audioEngine){
    const loop_grid = document.querySelector(".loop_grid");
    const pad_list = Array.from(document.querySelectorAll(".loop_pad"));

    loop_grid.addEventListener("click", (event) => {
        const loop_pad = event.target;

        if (loop_pad.classList.contains("loop_pad")){
            const loop_id = pad_list.indexOf(event.target);

            if (loop_pad.classList.contains("active")){
                loop_pad.classList.remove("active");
                audioEngine.toggleLoop(loop_id);
            } else{
                loop_pad.classList.add("active");
                audioEngine.toggleLoop(loop_id);
            }
       }
    });

    const play_button = document.querySelector(".play_button");

    play_button.addEventListener("click", (event) => {
        audioEngine.context.resume();
        const button = event.target;
        if (audioEngine.paused){
            play_button.innerText = "Pause";
            audioEngine.resumeAll();
        } else{
            play_button.innerText = "Play";
            audioEngine.pauseAll();
        }
    });

    const stop_button = document.querySelector(".stop_button");

    stop_button.addEventListener("click", () => {
        for (let i = 0; i < pad_list.length; i++) {
            const pad = pad_list[i];
        
            if (pad.classList.contains("active") || pad.classList.contains("wait_playing")) {
                pad.classList.remove("active", "wait_playing", "wait_stopped");
                audioEngine.toggleLoop(i);
        }
        }
        audioEngine.pauseAll();
        play_button.innerText = "Play";
    });

    function updateGrid(){
        audioEngine.updateStates();
        
        for (let i = 0; i < pad_list.length; i++){
            pad_list[i].classList.remove("active", "wait_playing", "wait_stopped");
            if (audioEngine.gridState[i] == "PLAYING" && audioEngine.paused){
                pad_list[i].classList.add("wait_playing");
            } else if(audioEngine.gridState[i] == "PLAYING"){
                pad_list[i].classList.add("active");
            } else if(audioEngine.gridState[i] == "WAIT_PLAYING"){
                pad_list[i].classList.add("wait_playing");
            } else if(audioEngine.gridState[i] == "WAIT_STOPPED"){
                pad_list[i].classList.add("wait_stopped");
            }
        }

        requestAnimationFrame(updateGrid);
    }

    updateGrid();
}