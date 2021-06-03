import React, { useState, useEffect } from "react";
import { Form, Button, Row, Col } from "react-bootstrap";
import styled from "styled-components";

const Login = (props) => {
  const [show, setShow] = useState(false);
  const handleChange = (e) => {
    props.setMqtt({ ...props.mqtt, [e.target.name]: e.target.value });
  };

  const handleValidation = () => {
    let formIsValid = true;
    let data = props.mqtt;

    if (!data["broker"]) {
      formIsValid = false;
    } else if (!data["broker"].startsWith("ws://")) {
      formIsValid = false;
    }

    if (!data["username"]) {
      formIsValid = false;
    }

    if (!data["password"]) {
      formIsValid = false;
    }

    if (formIsValid === false) {
      setShow(true);
    }
    return formIsValid;
  };

  const handleSubmit = (e) => {
    e.preventDefault();

    if (handleValidation()) {
      props.setToken("token123");
      props.setMqtt(props.mqtt);
    }
  };

  return (
    <Fragment>
      <h1>Log in first!</h1>
      {show && (
        <p className="error">
          Make sure all fields are filled in and broker URL starts with ws://
          !!!
        </p>
      )}
      <Form onSubmit={handleSubmit} className="mx-4">
        <Col>
          <Row>
            <Form.Label>Broker URL</Form.Label>
            <Form.Control
              name="broker"
              size="text"
              value={props.mqtt.broker}
              placeholder="ws://localhost:8080"
              onChange={handleChange}
            />
          </Row>
          <br />
          <Row>
            <Form.Label>Username</Form.Label>
            <Form.Control
              name="username"
              size="text"
              value={props.mqtt.username}
              placeholder="siting"
              onChange={handleChange}
            />
          </Row>
          <br />
          <Row>
            <Form.Label>Password</Form.Label>
            <Form.Control
              name="password"
              size="text"
              value={props.mqtt.password}
              placeholder="password"
              onChange={handleChange}
            />
          </Row>
        </Col>
        <br />
        <Button variant="light" type="submit">
          Submit
        </Button>
      </Form>
    </Fragment>
  );
};

export default Login;

const Fragment = styled.div`
  background-color: #c28484;
  width: 100%;
  height: 100vh;
  position: relative;
  padding: 10%;
  text-align: center;

  .error {
    padding: 0.5em;
    margin: 0.5em;
  }
`;
