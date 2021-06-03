import React from "react";
import { Navbar, Nav, Container, Button } from "react-bootstrap";
import Toggle from "./Toggler";
import Battery from "./Battery";
import SignalStrength from "./SignalStrength";
import Rover from "../assets/rover.svg";
import styled from "styled-components";

const HomeNavBar = (props) => {
  const handleClick = () => {
    props.deleteToken();
    props.deleteMqtt();
  };

  return (
    <Fragment>
      <Navbar bg="dark" variant="dark" className="justify-content-between">
        <Container>
          <Navbar.Brand href="/">
            <img className="img" alt="roverlogo" src={Rover} />
          </Navbar.Brand>
          <Nav className="mr-auto">
            <Nav.Link href="/">Home</Nav.Link>
            <Nav.Link href="/target">Target</Nav.Link>
            <Nav.Link href="/discrete">Discrete</Nav.Link>
          </Nav>
          <Navbar.Collapse className="justify-content-end">
            <Battery />
            <SignalStrength />
            <Toggle theme={props.theme} toggleTheme={props.toggleTheme} />
            <Button
              className="button"
              variant="outline-light"
              onClick={handleClick}
            >
              Log Out
            </Button>
          </Navbar.Collapse>
        </Container>
      </Navbar>
    </Fragment>
  );
};

export default HomeNavBar;

const Fragment = styled.div`
  background: #fff;

  .img {
    height: 55px;
    width: 55px;
    margin: 1px;
  }

  .button {
    margin: 1rem;
  }
`;
