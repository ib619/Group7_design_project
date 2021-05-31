import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
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

  // on message update
  useEffect(() => {
    if (message && message.topic === "obstacle/result") {
      setObstacles(JSON.parse(message.message));
    }
  }, [message, setObstacles]);

  // on position update
  useEffect(() => {
    if (message && message.topic === "position/update") {
      setPos(JSON.parse(message.message));
    }
  }, [message, setPos]);

  // on path update
  useEffect(() => {
    if (message && message.topic === "path") {
      if (show === true) {
        setPath(JSON.parse(message.message));
      }
    }
  }, [message, show, setPath]);

  return (
    <>
      <Border className="map">
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
        <MapInteractionCSS showControls={true} minScale={0.2} maxScale={2}>
          <Center src={home} />
          <Person src={pointerwhite} pos={pos} />
          {obstacles &&
            Array.from(obstacles).map((data, i) => (
              <Obstacle key={i} coords={data} />
            ))}
          {show &&
            Array.from(path).map((data, i) => <Point key={i} coords={data} />)}
          {/* {show &&
            Array.from(path).map((data, i, arr) => {
              let prevCoord = i === 0 ? [0, 0] : arr[i - 1];
              return (
                <Line
                  key={i}
                  x0={prevCoord[0]}
                  y0={prevCoord[1]}
                  x1={data[0]}
                  y1={data[1]}
                />
              );
            })} */}
          {/* <Line x0={0} y0={0} x1={100} y1={100} /> */}
          {/* <Line
            from={{ x: 0, y: 0 }}
            to={{ x: 100, y: 100 }}
            style="5px solid white"
          /> */}
        </MapInteractionCSS>
      </Border>
      <Coords>
        x: {pos.x}, y: {pos.y}, direction: {pos.heading}˚
      </Coords>
    </>
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
  background: #320202;

  .switch {
    position: absolute;
    z-index: 2;
  }
`;

const Center = styled.img`
  position: relative;
  left: 380px;
  bottom: -230px;
`;

const Obstacle = styled.div.attrs((props) => ({
  style: {
    left: props.coords[1] + 380,
    bottom: props.coords[2] - 200,
    opacity: 1 / props.coords[3],
    background: props.coords[0] === "blue" ? "turquoise" : props.coords[0],
  },
}))`
  padding: 0.5rem;
  width: 15px;
  height: 15px;
  position: relative;
  transition: all 1s ease-out;
`;

const Point = styled.div.attrs((props) => ({
  style: {
    left: props.coords[0] + 380,
    bottom: props.coords[1] - 200,
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
    left: props.pos.x + 380, // hardcoded offset values :(
    bottom: props.pos.y - 180,
  },
}))`
  position: relative;
  height: 50px;
  width: 40px;
  z-index: 1;
  transform: rotate(${({ pos }) => pos.heading}deg);
  transition: all 0.6s ease-out;
`;

const Coords = styled.p`
  background: ${({ theme }) => theme.background};
  color: ${({ theme }) => theme.text};
  text-align: center;
  font-size: 15px;
  padding: 0.5rem;
`;
