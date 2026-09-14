export function update_beat_counter(audioEngine) {
    const leds = document.querySelectorAll('.beat_led');
    
    if (!audioEngine.paused) {
        const beatDuration = 60 / audioEngine.tempo;
        const timePassed = audioEngine.context.currentTime - audioEngine.startTime;
        const currentBeat = Math.floor(timePassed / beatDuration) % 4;

        for (let i = 0; i < leds.length; i++) {
            leds[i].classList.toggle('active', i == currentBeat);
        }
    } else {
        for (let i = 0; i < leds.length; i++) {
            leds[i].classList.remove('active');
        }
    }

    requestAnimationFrame(() => update_beat_counter(audioEngine));
}