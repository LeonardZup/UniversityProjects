<?php
    require_once "db_connect.php";

    session_start();

    if (isset($_SESSION["logged"])){
        header("location: station.php");
        exit();
    }

    $mess = "";

    if ($_SERVER["REQUEST_METHOD"] === "POST"){
        $username = $_POST["username"];
        $password = $_POST["password"];

        if ($username != "" && $password != ""){
            try {
                $stmt = $pdo->prepare("SELECT * FROM users WHERE username = :username");     
                $stmt->execute(["username" => $username]);
                $user = $stmt->fetch();

                if ($user && password_verify($password, $user["password"])){
                    session_regenerate_id(true);
                    $_SESSION["logged"] = true;
                    $_SESSION["username"] = $user["username"];

                    header("location: station.php");
                    exit();
                } else{
                    $mess = "Wrong username or password";
                }
            } catch (PDOException $e){
                $mess = "An error occourred, try again";
            }
        } else{
            $mess = "Insert all values";
        }
    }
?>

<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <title>LOOP_STTN</title>
        <link rel="stylesheet" href="../css/colors.css">
        <link rel="stylesheet" href="../css/main.css">
        <link rel="stylesheet" href="../css/access.css">
        <link rel="stylesheet" href="../css/fonts.css">
    </head>
    <body>
        <header>
            <a href="home.php" class="home_link">LOOP_STTN :// V1.0</a>
        </header>
        <div class="form_container">
            <h2>Login</h2>
            <div class="error_container">
                <?php if ($mess): ?>
                    <span>[ ERROR: <?php echo htmlspecialchars($mess); ?> ]</span>
                <?php endif; ?>
            </div>
            <form id="form" method="POST">
                <div id="username_container" class="form_elem">
                    <label for="username">Username</label>
                    <input type="text" id="username" name="username" placeholder="USER_ID" required>
                </div>
                <div id="password_container" class="form_elem">
                    <label for="password">Password</label>
                    <input type="password" id="password" name="password" placeholder="*****" required>
                </div>
                <div id="button_container" class="form_elem">
                    <button id="login_button" type="submit">Login</button>
                </div>
            </form>
            <div id="signup_container" class="switch_signup">
                <span>Don't have an account?</span>
                <a href="signup.php" class="enter_link">Sign up</a>
            </div>
        </div>
        <footer>
            <a href="../html/guide.html" class="footer_elem">Guide</a>
        </footer>
    </body>
</html>