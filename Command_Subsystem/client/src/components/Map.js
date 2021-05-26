import React, { useState, useEffect } from "react";
import styled from "styled-components";
import { useSubscription } from "mqtt-react-hooks";

const Map = (props) => {
  const { message } = useSubscription("obstacle/result");
  const [obstacles, setObstacles] = useState([]);

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

  return (
    <>
      <Border>
        <Block pos={props.pos} />
        {obstacles &&
          obstacles.map((data, index) => (
            <Obstacle key={index} coords={data} />
          ))}
      </Border>
      <Coords>
        x: {props.pos.x}, y: {props.pos.y}
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

const Block = styled.div.attrs((props) => ({
  style: {
    left: props.pos.x,
    top: props.pos.y,
  },
}))`
  background: ${({ theme }) => theme.background};
  color: black;
  padding: 0.5rem;
  width: 1rem;
  height: 1rem;
  position: relative;
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

const Coords = styled.p`
  background: ${({ theme }) => theme.background};
  color: ${({ theme }) => theme.text};
  text-align: center;
  font-size: 15px;
  padding: 0.5rem;
`;
