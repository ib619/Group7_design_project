const express = require("express");
const router = express.Router();
const Obstacles = require("../models/obstacles");

// request: GET all articles
router.get("/", (req, res) => {
  Obstacles.find()
    .then((obstacle) => res.json(obstacle))
    .catch((err) => res.status(400).json(`Error: ${err}`));
});

// request: POST new article
router.post("/add", (req, res) => {
  const newObstacle = new Obstacles({
    x: req.body.x,
    y: req.body.y,
  });

  newObstacle
    .save()
    .then(() => res.json("Obstacle has been created successfully."))
    .catch((err) => res.status(400).json(`Error: ${err}`));
});

// request: FIND article by ID
router.get("/:id", (req, res) => {
  Obstacles.findById(req.params.id)
    .then((obstacle) => res.json(obstacle))
    .catch((err) => res.status(400).json(`Error: ${err}`));
});

// request: FIND article by ID and update
router.put("/update/:id", (req, res) => {
  Obstacles.findById(req.params.id)
    .then((obstacle) => {
      obstacle.x = req.body.x;
      obstacle.y = req.body.y;

      obstacle
        .save()
        .then(() => res.json("obstacle has been updated successfully."))
        .catch((err) => res.status(400).json(`Error: ${err}`));
    })
    .catch((err) => res.status(400).json(`Error: ${err}`));
});

// request: FIND article by ID and delete
router.delete("/:id", (req, res) => {
  Obstacles.findByIdAndDelete(req.params.id)
    .then(() => res.json("obstacle has been deleted successfully."))
    .catch((err) => res.status(400).json(`Error: ${err}`));
});

module.exports = router;
