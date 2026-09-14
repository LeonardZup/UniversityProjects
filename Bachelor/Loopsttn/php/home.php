<?php
    session_start();

    if (isset($_SESSION["logged"]) && $_SESSION["logged"] === true) {
        $username = htmlspecialchars($_SESSION["username"], ENT_QUOTES, "UTF-8");
        $user_element = "<a href='logout.php' class='user_logout'>" . $username . "</a>";
    } else {
        $user_element = "<span class='user_status'>NOT LOGGED</span>";
    }
?>
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="utf-8">
        <title>LOOP_STTN</title>
        <link rel="stylesheet" href="../css/colors.css">
        <link rel="stylesheet" href="../css/main.css">
        <link rel="stylesheet" href="../css/home.css">
        <link rel="stylesheet" href="../css/fonts.css">
    </head>
    <body>
        <header>
            <a href="home.php" class="home_link">LOOP_STTN :// V1.0</a>
            <?php echo $user_element; ?>
        </header>
        
        <main class="enter_container">
            <a href="login.php" class="enter_link">Start</a>
        </main>
        <footer>
            <a href="../html/guide.html" class="footer_elem">Guide</a>
        </footer>
    </body>
</html>