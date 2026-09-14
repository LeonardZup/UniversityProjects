<?php
    require_once "db_connect.php";

    session_start();

    if (!isset($_SESSION["logged"]) || $_SESSION["logged"] != true){
        header("location: login.php");
        exit();
    }
    $username = $_SESSION["username"];
?>

<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <title>LOOP_STTN</title>
        <link rel="stylesheet" href="../css/colors.css">
        <link rel="stylesheet" href="../css/main.css">
        <link rel="stylesheet" href="../css/loop_grid.css">
        <link rel="stylesheet" href="../css/gain_slider.css">
        <link rel="stylesheet" href="../css/visualizer_bar.css">
        <link rel="stylesheet" href="../css/freq_visualizer.css">
        <link rel="stylesheet" href="../css/beat_counter.css">
        <link rel="stylesheet" href="../css/xy_pad.css">
        <link rel="stylesheet" href="../css/strip_pad.css">
        <link rel="stylesheet" href="../css/station.css">
        <link rel="stylesheet" href="../css/fonts.css">
        <script type="module" src="../js/station.js"></script>
    </head>
    <body>
        <header>
            <a href="home.php" class="home_link">LOOP_STTN :// V1.0</a>
            <a href="logout.php" class="user_logout"><?php echo htmlspecialchars($username, ENT_QUOTES, "UTF-8"); ?></a>
        </header>
        <div class="station_container">
            <div class="controls">
                <button class="play_button">Play</button>
                <button class="stop_button">Stop</button>
                <div class="beat_counter">
                    <div class="beat_led"></div>
                    <div class="beat_led"></div>
                    <div class="beat_led"></div>
                    <div class="beat_led"></div>
                </div>
            </div>
            <div class="options">
                <button class="save_button">Save</button>
                <button class="load_button">Load</button>
                <span class="bpm_display"></span>
                <select class="set_selection"></select>
            </div>

            <div class="loop_grid">
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
                <div class="loop_pad"><span class="led"></span><span class="name"></span></div>
            </div>
            <div class="filter_pad xy_pad">
                <div class="pad_area">
                    <span class="xy_axis_label top-left">Resonance</span>
                    <span class="xy_axis_label bottom-right">Cutoff</span>
                    <div class="filter_cursor xy_cursor"></div>
                </div>
                <span class="xy_label">Filter</span>
            </div>
            <div class="delay_pad xy_pad">
                <div class="pad_area">
                    <span class="xy_axis_label top-left">Feedback</span>
                    <span class="xy_axis_label bottom-right">Time</span>
                    <div class="delay_cursor xy_cursor"></div>
                </div>
                <span class="xy_label">Delay</span>
            </div>
            <div class="stutter_pad strip_pad">
                <div class="pad_area">
                    <div class="stutter_stage strip_stage"></div>
                    <div class="stutter_stage strip_stage"></div>
                    <div class="stutter_stage strip_stage"></div>
                    <div class="stutter_stage strip_stage"></div>
                    <div class="stutter_cursor strip_cursor"></div>
                </div>
                <span class="strip_label">Stutter</span>
            </div>
            <div class="roll_pad strip_pad">
                <div class="pad_area">
                    <div class="roll_stage strip_stage"></div>
                    <div class="roll_stage strip_stage"></div>
                    <div class="roll_stage strip_stage"></div>
                    <div class="roll_stage strip_stage"></div>
                    <div class="roll_cursor strip_cursor"></div>
                </div>
                <span class="strip_label">Roll</span>
            </div>
            <div class="freq_visualizer">
                <canvas class="freq_canvas"></canvas>
            </div>
            <div class="gain_slider">
                <span class="label_gain">Gain</span>
                <div class="gain_cursor"></div>
            </div>
            <div class="visualizer_bar">
                <span class="label_left_visualizer">L</span>
                <span class="label_right_visualizer">R</span>
                <canvas class="left_visualizer_bar"></canvas>
                <canvas class="right_visualizer_bar"></canvas>
            </div>
        </div>
        <footer>
            <a href="../html/guide.html" class="footer_elem">Guide</a>
        </footer>
        <div class="loading">
            <span>Loading...</span>
        </div>
        <div class="save_overlay">
            <div class="save_menu">
                <div class="menu_header">
                    <span>Save preset</span>
                    <button class="close_save">X</button>
                </div>
                <form class="save_form">
                    <div class="error_message"></div>
                    <input type="text" class="preset_name" placeholder="PRESET_NAME" maxlength="20" required>
                    <button id="confirm_save" type="submit">Confirm</button>
                </form>
            </div>
        </div>
        <div class="load_overlay">
            <div class="load_menu">
                <div class="menu_header">
                    <span>Your presets</span>
                    <button class="close_load">X</button>
                </div>
                <div class="load_list"></div>
            </div>
        </div>
    </body>
</html>