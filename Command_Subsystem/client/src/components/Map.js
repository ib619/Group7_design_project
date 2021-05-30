import React, { useState, useEffect } from "react";
import styled from "styled-components";
import { useSubscription } from "mqtt-react-hooks";
// import pointerblack from "../assets/pointer-black.svg";
import pointerwhite from "../assets/pointer-white.svg";
import { MapInteractionCSS } from "react-map-interaction";
import Plus from "../assets/plus-solid.svg";

const Map = () => {
  // for the map stuff
  const [pos, setPos] = useState({ x: 0, y: 0, heading: 0 });
  const [obstacles, setObstacles] = useState([]);
  const { message } = useSubscription(["obstacle/result", "position/update"]);

  // on refresh
  useEffect(() => {
    const data = JSON.parse(localStorage.getItem("obstacles"));
    setObstacles(data);
  }, []);

  // when message arrived
  useEffect(() => {
    if (message && message.topic === "obstacle/result") {
      setObstacles(JSON.parse(message.message));
      localStorage.setItem("obstacles", JSON.stringify(message.message));
    }
  }, [message, setObstacles]);

  // on position update
  useEffect(() => {
    if (message && message.topic === "position/update") {
      setPos(JSON.parse(message.message));
    }
  }, [message, setPos]);

  return (
    <>
      <Border>
        <MapInteractionCSS>
          <Center src={Plus} />
          <Pointer src={pointerwhite} pos={pos} />
          {obstacles &&
            obstacles.map((data, index) => (
              <Obstacle key={index} coords={data} />
            ))}
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
  border: 2px solid grey;
`;

const Center = styled.img`
  color: white;
  position: relative;
  left: 380px;
  bottom: -230px;
`;

const Obstacle = styled.div.attrs((props) => ({
  style: {
    left: props.coords[1] + 380,
    bottom: props.coords[2] - 200,
    opacity: 1 / props.coords[3],
    background: props.coords[0],
  },
}))`
  padding: 0.5rem;
  width: 15px;
  height: 15px;
  position: relative;
  transition: all 1s ease-out;
`;

const Pointer = styled.img.attrs((props) => ({
  style: {
    left: props.pos.x + 380, // hardcoded offset values :(
    bottom: props.pos.y - 180,
  },
}))`
  position: relative;
  height: 50px;
  width: 40px;
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
