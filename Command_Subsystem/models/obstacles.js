const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const obstacleSchema = new Schema({
  x: { type: Number, required: true },
  y: { type: Number, required: true },
});

const Obstacles = mongoose.model("Obstacles", obstacleSchema);

module.exports = Obstacles;
