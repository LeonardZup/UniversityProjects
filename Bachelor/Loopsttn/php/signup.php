<?php
    require_once "db_connect.php";

    session_start();

    if (isset($_SESSION["logged"])){
        header("location: station.php");
        exit();
    }

    $mess = "";

    define("USERNAME_REGEX", "/^[a-zA-Z0-9._]{4,12}$/");
    define("PASSWORD_REGEX", "/^(?=.*[a-z])(?=.*[A-Z])(?=.*\d).{8,64}$/");

    if ($_SERVER["REQUEST_METHOD"] === "POST"){
        $username = $_POST["username"];
        $password = $_POST["password"];
        $rep_password = $_POST["rep_password"];

        if ($username != "" && $password != "" && $rep_password != ""){
            if (!preg_match(USERNAME_REGEX, $username) || !preg_match(PASSWORD_REGEX, $password)){
                $mess = "Invalid username or password";
            } else if ($password == $rep_password){
                try {
                    $stmt = $pdo->prepare("SELECT * FROM users WHERE username = :username");     
                    $stmt->execute(["username" => $username]);
                    
                    if ($stmt->fetch()){
                        $mess = "Username already exists";
                    } else{
                        $hashed_password = password_hash($password, PASSWORD_DEFAULT);
                        $ins = $pdo->prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
                        $ins->execute(["username" => $username, "password" => $hashed_password]);

                        $_SESSION["logged"] = true;
                        $_SESSION["username"] = $username;
                        session_regenerate_id(true);

                        header("location: station.php");
                        exit();
                    }
                } catch (PDOException $e){
                    $mess = "An error occourred, try again";
                }
            } else{
                $mess = "Repeated password is incorrect";
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
            <h2>Sign up</h2>
            <div class="error_container">
                <?php if ($mess): ?>
                    <span>[ ERROR: <?php echo htmlspecialchars($mess); ?> ]</span>
                <?php endif; ?>
            </div>
            <form id="form" method="POST">
                <div id="username_container" class="form_elem">
                    <div class="label_container">
                        <label for="username">Username</label>
                        <span class="input_format">4-12 chars</span>
                    </div>
                    <input type="text" id="username" name="username" placeholder="USER_ID" required>
                </div>
                <div id="password_container" class="form_elem">
                    <div class="label_container">
                        <label for="password">Password</label>
                        <span class="input_format">Min 8 chars, 1 upper case, 1 number</span>
                    </div>
                    <input type="password" id="password" name="password" placeholder="*****" required>
                </div>
                <div id="rep_password_container" class="form_elem">
                    <label for="rep_password">Repeat password</label>
                    <input type="password" id="rep_password" name="rep_password" placeholder="*****" required>
                </div>
                <div id="button_container" class="form_elem">
                    <button id="login_button" type="submit">Sign up</button>
                </div>
            </form>
            <div id="signup_container" class="switch_signup">
                <span>Already have an account?</span>
                <a href="login.php" class="enter_link">Login</a>
            </div>
        </div>
        <footer>
            <a href="../html/guide.html" class="footer_elem">Guide</a>
        </footer>
    </body>
</html>