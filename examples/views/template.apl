<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%%Title%%</title>
</head>
<body>

    <header>
        <h1>%%Header%%</h1>
    </header>

    <main>
        <p>%%Content%%</p>
        <form action="/post" method="post">
          <label for="fname">First name:</label><br>
          <input type="text" id="fname" name="fname" value="John"><br>
          <label for="lname">Last name:</label><br>
          <input type="text" id="lname" name="lname" value="Doe"><br><br>
          <input type="radio" id="html" name="fav_language" value="HTML">
          <label for="html">HTML</label><br>
          <input type="radio" id="css" name="fav_language" value="CSS">
          <label for="css">CSS</label><br>
          <input type="radio" id="javascript" name="fav_language" value="JavaScript">
          <label for="javascript">JavaScript</label>
          <input type="checkbox" id="vehicle1" name="vehicle1" value="Bike">
          <label for="vehicle1"> I have a bike</label><br>
          <input type="checkbox" id="vehicle2" name="vehicle2" value="Car">
          <label for="vehicle2"> I have a car</label><br>
          <input type="submit" value="Submit">        
        </form> 
    </main>

    <footer>
        <p>&copy; 2025 %%Footer%%</p>
    </footer>

</body>
</html>

