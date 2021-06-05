import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
import GridLines from "react-gridlines";
import { MapInteractionCSS } from "react-map-interaction";
import BootstrapSwitchButton from "bootstrap-switch-button-react";
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

  // // on refresh
  // useEffect(() => {
  //   const obstacle = JSON.parse(localStorage.getItem("obstacles"));
  //   if (obstacle !== null) {
  //     setObstacles(obstacle);
  //   }
  // }, [setObstacles]);

  // on message update
  useEffect(() => {
    if (message) {
      if (message.topic === "obstacle/result") {
        // on obstacle update
        setObstacles(JSON.parse(message.message));
        // localStorage.setItem("obstacles", message.message);
      } else if (message.topic === "position/update") {
        // on position update
        setPos(JSON.parse(message.message));
      } else if (message.topic === "path") {
        // on path update
        setPath(JSON.parse(message.message));
      }
    }
  }, [message, setObstacles, setPos, setPath]);

  return (
    <React.Fragment>
      <Border>
        <BootstrapSwitchButton
          checked={false}
          width={120}
          onstyle="light"
          offstyle="dark"
          onlabel="hide path"
          offlabel="show path"
          className="switch"
          onChange={(checked) => {
            setShow(checked);
          }}
        />
        <MapInteractionCSS
          showControls={true}
          minScale={0.5}
          maxScale={2}
          translationBounds={{ xMin: -495, xMax: 1250, yMin: -788, yMax: 1240 }}
          // value={map}
          // onChange={(val) => {
          //   setMap(val);
          // }}
        >
          <GridLines cellWidth={50} className="body">
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
    display: flex;
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
    left: props.coords[1] + 2810,
    bottom: props.coords[2] - 2740,
    opacity: 1 / props.coords[3],
    background: props.coords[0] === "blue" ? "turquoise" : props.coords[0],
  },
}))`
  width: 15px;
  height: 15px;
  position: relative;
  transition: all 1s ease-out;
`;

// left: 390
// bottom: -155

const Point = styled.div.attrs((props) => ({
  style: {
    left: props.coords[0] + 370,
    bottom: props.coords[1] - 155,
  },
}))`
  background: white;
  width: 10px;
  height: 10px;
  border-radius: 50%;
  position: relative;
  transition: all 1s ease-out;
`;

const Person = styled.img.attrs((props) => ({
  style: {
    left: props.pos.x + 2840, // hardcoded offset values :(
    bottom: props.pos.y - 2700,
  },
}))`
  position: relative;
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
