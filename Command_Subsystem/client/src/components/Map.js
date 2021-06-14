import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
import GridLines from "react-gridlines";
import { Button } from "react-bootstrap";
import { MapInteractionCSS } from "react-map-interaction";
import styled from "styled-components";
import pointerwhite from "../assets/pointer-white.svg";
import home from "../assets/home.svg";
// import plus from "../assets/plus-solid.svg";

const Map = () => {
  // for the map stuff
  const [pos, setPos] = useState({ x: 0, y: 0, heading: 0 });
  const [obstacles, setObstacles] = useState({});
  const [show, setShow] = useState(false);
  const [path, setPath] = useState({});
  const { message } = useSubscription([
    "obstacle/result",
    "position/update",
    "path",
  ]);

  // for the map zooming features
  // const [map, setMap] = useState({ scale: 1, translation: { x: 0, y: 0 } });

  // on message update
  useEffect(() => {
    if (message) {
      let data = JSON.parse(message.message);
      if (message.topic === "obstacle/result") {
        // on obstacle update
        setObstacles(data);
      } else if (message.topic === "position/update") {
        // on position update
        setPos(data);
      } else if (message.topic === "path") {
        // on path update
        setPath(data);
      }
    }
  }, [message, setObstacles, setPos]);

  useEffect(() => {
    console.log(show);
    console.log(path);
  }, [show]);

  return (
    <React.Fragment>
      <Border>
        <Button
          type="checkbox"
          variant="secondary"
          checked={show}
          className="switch"
          // value="1"
          onClick={(e) => {
            setShow(!show);
          }}
        >
          Show Path
        </Button>
        <MapInteractionCSS
          showControls={true}
          // minScale={0.5}
          maxScale={2}
          // translationBounds={{ xMin: -495, xMax: 1250, yMin: -788, yMax: 1240 }}
          // value={map}
          // onChange={(val) => {
          //   setMap(val);
          // }}
        >
          <GridLines lineColor="grey" cellWidth={50} className="body">
            <Center src={home} />
            <Person src={pointerwhite} pos={pos} />
            {obstacles &&
              Array.from(obstacles).map((data, i) => (
                <Obstacle key={i} coords={data} />
              ))}
            {show &&
              Array.from(path).map((data, i) => (
                <Point key={i} coords={data} />
              ))}
          </GridLines>
        </MapInteractionCSS>
      </Border>

      <Coords>
        x: {pos.x}, y: {pos.y}, direction: {pos.heading}˚
      </Coords>
    </React.Fragment>
  );
};

export default Map;

const Border = styled.div`
  position: relative;
  margin: 1rem auto;
  width: 800px;
  height: 500px;
  padding: 0.5rem;
  border: 2px solid #200000;
  border-width: 10px;
  background: ${({ theme }) => theme.map};

  .switch {
    position: absolute;
    z-index: 1;
  }

  .body {
    height: 5000px;
    width: 5000px;
    position: relative;
    right: 2500px;
    bottom: 2500px;
  }
`;

const Center = styled.img`
  position: relative;
  left: 2880px;
  bottom: -2730px;
  height: 40px;
  width: 40px;
`;

// left: 380px;
// bottom: -230px;

const Obstacle = styled.div.attrs((props) => ({
  style: {
    position: "absolute",
    left: props.coords[1] * 0.2 + 2900,
    bottom: props.coords[2] * 0.2 + 2243,
    opacity: 1 / props.coords[3],
    background: props.coords[0] === "blue" ? "turquoise" : props.coords[0],
  },
}))`
  position: absolute;
  width: 15px;
  height: 15px;
  transition: all 1s ease-out;
`;

// left: 2810
// bottom: -2740

const Point = styled.div.attrs((props) => ({
  style: {
    left: props.coords[0] * 0.2 + 2880,
    bottom: props.coords[1] * 0.2 + 2230,
  },
}))`
  background: white;
  width: 10px;
  height: 10px;
  border-radius: 50%;
  position: absolute;
  transition: all 1s ease-out;
`;

const Person = styled.img.attrs((props) => ({
  style: {
    left: props.pos.x * 0.2 + 2878, // hardcoded offset values :(
    bottom: props.pos.y * 0.2 + 2220,
  },
}))`
  position: absolute;
  height: 50px;
  width: 40px;
  z-index: 1;
  transform: rotate(${({ pos }) => pos.heading}deg);
  transition: all 0.6s ease-out;
`;

// left: 340
// bottom: -200

const Coords = styled.p`
  background: ${({ theme }) => theme.background};
  color: ${({ theme }) => theme.text};
  text-align: center;
  font-size: 15px;
  padding: 0.5rem;
`;
