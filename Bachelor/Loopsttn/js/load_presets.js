import { updateLoopGrid } from "./station.js";

export function load_menu_setup(audioEngine, sets) {
    const loadOverlay = document.querySelector(".load_overlay");
    const loadList = document.querySelector(".load_list");
    const loadButton = document.querySelector(".load_button");
    const closeButton = document.querySelector(".close_load");
    const loading = document.querySelector(".loading");

    closeButton.onclick = () => loadOverlay.classList.remove("open");

    loadOverlay.addEventListener("click", (e) => {
        if (e.target == loadOverlay) {
            loadOverlay.classList.remove("open");
        }
    });

    loadButton.addEventListener("click", async () => {
        loadOverlay.classList.add("open");

        try {
            const response = await fetch("load_presets.php");
            const result = await response.json();

            if (result.status == "success") {
                displayList(result.presets);
            } else {
                loadList.innerText = "[ Error: ${result.message} ]";
            }
        } catch (err) {
            loadList.innerText = "[ Error: Connection error ]";
        }
    });

    function displayList(presets) {
        loadList.innerText = "";
        while (loadList.firstChild) {
            loadList.firstChild.remove();
        }

        if (presets.length == 0) return;

        for (let i = 0; i < presets.length; i++) {
            const p = presets[i];
            const elem = document.createElement("div");
            elem.className = "preset_elem";

            const elemName = document.createElement("span");
            elemName.innerText = p.preset_name + " ";

            const elemPreset = document.createElement("span");
            elemPreset.innerText = `(${p.loop_set})`;
            elemPreset.classList.add("preset_elem_type");
            elemName.appendChild(elemPreset);

            const deleteButton = document.createElement("button");
            deleteButton.className = "delete_preset";
            deleteButton.innerText = "X";

            elem.appendChild(elemName);
            elem.appendChild(deleteButton);

            elem.addEventListener("click", (e) => {
                if (e.target.classList.contains("delete_preset")) return;
                loadPreset(p);
            });

            deleteButton.onclick = () => deletePreset(p.preset_name, elem);
            loadList.appendChild(elem);
        }
    }

    async function loadPreset(p) {
        loading.style.display = "flex";
        loadOverlay.classList.remove("open");

        let setConfig = null;
        for (let i = 0; i < sets.length; i++) {
            if (sets[i].name == p.loop_set) {
                setConfig = sets[i];
                break;
            }
        }
        
        if (!setConfig) return;

        await audioEngine.loadSet(setConfig);
        audioEngine.startTime = audioEngine.context.currentTime;

        const pads = document.querySelectorAll(".loop_pad");
        for (let i = 0; i < pads.length; i++) {
            pads[i].classList.remove("active", "wait_playing", "wait_stopped");
        }

        for (let i = 0; i < audioEngine.gridState.length; i++) {
            audioEngine.gridState[i] = "STOPPED";
        }

        const activeIndices = JSON.parse(p.active_loops);
        for (let i = 0; i < activeIndices.length; i++) {
            const index = activeIndices[i];
            if (pads[index]) {
                pads[index].classList.add("active");
                audioEngine.gridState[index] = "PLAYING";
            }
        }

        updateLoopGrid(setConfig, sets);
        
        loading.style.display = "none";
    }

    async function deletePreset(name, element) {
        try {
            const response = await fetch("delete_preset.php", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ presetName: name })
            });
            const result = await response.json();

            if (result.status == "success") {
                element.remove();
            } else {
                alert("Error: " + result.message);
            }
        } catch (err) {
            alert("Connection error");
        }
    }
}