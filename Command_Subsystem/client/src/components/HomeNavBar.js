import React from "react";
import { Navbar, Nav, Container } from "react-bootstrap";
import Toggle from "./Toggler";
import Battery from "./Battery";
import SignalStrength from "./SignalStrength";

const HomeNavBar = (props) => {
  return (
    <>
      <Navbar bg="dark" variant="dark">
        <Container>
          <Navbar.Brand href="/">Rover</Navbar.Brand>
          <Nav className="mr-auto">
            <Nav.Link href="/">Home</Nav.Link>
            <Nav.Link href="/target">Target</Nav.Link>
            <Nav.Link href="/discrete">Discrete</Nav.Link>
          </Nav>
          <Navbar.Collapse className="justify-content-end">
            <Battery />
            <SignalStrength />
            <Toggle theme={props.theme} toggleTheme={props.toggleTheme} />
          </Navbar.Collapse>
        </Container>
      </Navbar>
    </>
  );
};

export default HomeNavBar;
