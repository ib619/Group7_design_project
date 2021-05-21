const express = require("express");
const mongoose = require("mongoose");
const cors = require("cors");
require("dotenv").config();

const app = express();
const port = process.env.PORT || 5000;

app.use(cors());
app.use(express.json());

const uri = process.env.ATLAS_URI;

mongoose.connect(uri, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});

const connection = mongoose.connection;
connection.once("open", () => console.log("Connection is successful!"));

const obstacleRouter = require("./routes/obstacles");
app.use("/obstacles", obstacleRouter);

app.listen(port, () => console.log(`The app is running on Port: ${port}.`));
