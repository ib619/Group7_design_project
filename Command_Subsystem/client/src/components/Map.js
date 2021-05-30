import React, { useState, useEffect } from "react";
import styled from "styled-components";
import { useSubscription } from "mqtt-react-hooks";
// import pointerblack from "../assets/pointer-black.svg";
import pointerwhite from "../assets/pointer-white.svg";

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
      localStorage.setItem("obstacles", JSON.stringify(obstacles));
    }
  }, [message, obstacles]);

  // on position update
  useEffect(() => {
    if (message && message.topic === "position/update") {
      console.log(message);
      setPos(JSON.parse(message.message));
    }
  }, [message, pos]);

  return (
    <>
      <Border>
        <Pointer src={pointerwhite} pos={pos} />
        {obstacles &&
          obstacles.map((data, index) => (
            <Obstacle key={index} coords={data} />
          ))}
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
  width: 26rem;
  height: 26rem;
  padding: 0.5rem;
  border: 2px solid black;
`;

const Obstacle = styled.div.attrs((props) => ({
  style: {
    left: props.coords[1],
    top: props.coords[2],
    opacity: 1 / props.coords[3],
    background: props.coords[0],
  },
}))`
  padding: 0.5rem;
  width: 1rem;
  height: 1rem;
  position: relative;
`;

const Pointer = styled.img.attrs((props) => ({
  style: {
    left: props.pos.x,
    top: props.pos.y,
  },
}))`
  position: relative;
  height: 45px;
  width: 45px;
  margin: 5px;
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
