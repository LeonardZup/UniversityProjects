export function save_menu_setup(audioEngine) {
    const saveOverlay = document.querySelector(".save_overlay");
    const saveForm = document.querySelector(".save_form");
    const saveButton = document.querySelector(".save_button");
    const closeButton = document.querySelector(".close_save");
    const presetInput = document.querySelector(".preset_name");
    const errorDisplay = document.querySelector(".error_message");

    saveButton.addEventListener("click", () => {
        saveOverlay.classList.add("open");
        errorDisplay.innerText = "";
        presetInput.focus();
    });

    const closeSave = () => {
        saveOverlay.classList.remove("open");
        presetInput.value = "";
        errorDisplay.innerText = "";
    };

    closeButton.onclick = closeSave;

    saveOverlay.addEventListener("click", (e) => {
        if (e.target == saveOverlay) {
            closeSave();
        }
    });

    saveForm.addEventListener("submit", async (e) => {
        e.preventDefault();

        const activeIndices = [];
        for (let i = 0; i < audioEngine.gridState.length; i++) {
            const state = audioEngine.gridState[i];
            if (state == "PLAYING" || state == "WAIT_PLAYING") {
                activeIndices.push(i);
            }
        }

        const payload = {
            presetName: presetInput.value.trim(),
            loopSet: audioEngine.config.name,
            activeLoops: activeIndices
        };

        try {
            const response = await fetch("save_preset.php", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });

            const result = await response.json();

            if (result.status == "success") {
                closeSave();
            } else {
                errorDisplay.innerText = "[ ERROR: " + result.message + " ]";
            }
        } catch (err) {
            errorDisplay.innerText = "[ ERROR: Server unreachable ]";
        }
    });
}