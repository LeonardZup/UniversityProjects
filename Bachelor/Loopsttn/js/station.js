import {AudioEngine} from "./audio_engine.js";
import {loop_grid_setup} from "./loop_grid.js";
import {filter_pad_setup} from "./filter_pad.js";
import {delay_pad_setup} from "./delay_pad.js"
import {stutter_pad_setup} from "./stutter_pad.js";
import {roll_pad_setup} from "./roll_pad.js";
import {gain_slider_setup} from "./gain_slider.js";
import {visualizer_bar_setup} from "./visualizer_bar.js";
import {visualizer_freq_setup} from "./visualizer_freq.js"
import {update_beat_counter} from "./beat_counter.js";
import {save_menu_setup} from "./save_preset.js";
import {load_menu_setup} from "./load_presets.js";

document.addEventListener("DOMContentLoaded", async () => {
    const audioEngine = new AudioEngine();
    const loading = document.querySelector(".loading");

    const response = await fetch("../loop_sets/sets_config.json");
    const sets = await response.json()

    const defaultSet = sets[0];
    
    await audioEngine.loadSet(defaultSet);
    updateLoopGrid(defaultSet, sets);

    fillSetsMenu();

    loading.style.display = "none";

    save_menu_setup(audioEngine);
    load_menu_setup(audioEngine, sets);

    loop_grid_setup(audioEngine);
    filter_pad_setup(audioEngine);
    delay_pad_setup(audioEngine);
    stutter_pad_setup(audioEngine);
    roll_pad_setup(audioEngine);
    gain_slider_setup(audioEngine);
    visualizer_bar_setup(audioEngine);
    visualizer_freq_setup(audioEngine);
    update_beat_counter(audioEngine);

    function fillSetsMenu(){
        const set_selection = document.querySelector(".set_selection");

        for (let i = 0; i < sets.length; i++){
            const option = document.createElement("option");
            option.value = i;
            option.innerText = sets[i].name;
            set_selection.appendChild(option);
        }

        set_selection.addEventListener("change", async (event) => {
            loading.style.display = "flex";

            const id = event.target.value;
            const newSet = sets[id];

            await audioEngine.loadSet(newSet);
            audioEngine.startTime = audioEngine.context.currentTime;

            updateLoopGrid(newSet);

            loading.style.display = "none";
        });
    }
});

export function updateLoopGrid(currentSet, sets) {
        const buttons = document.querySelectorAll(".loop_pad");

        for (let i = 0; i < buttons.length; i++) {
            const button = buttons[i];
            const loop = currentSet.loops[i];

            const buttonName = button.querySelector(".name");
            buttonName.innerText = loop.name;
        }

        const bpm = document.querySelector(".bpm_display");
        bpm.innerText = currentSet.tempo + " BPM";

        const play_button = document.querySelector(".play_button");
        play_button.innerText = "Play";

        if (sets) {
            const set_selection = document.querySelector(".set_selection");
    
            for (let i = 0; i < sets.length; i++) {
                if (sets[i].name == currentSet.name) {
                    set_selection.value = i;
                    break;
                }
            }
        }
    }