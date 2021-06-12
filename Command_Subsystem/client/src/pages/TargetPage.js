import React, { useState } from "react";
import Map from "../components/Map";
import { useMqttState } from "mqtt-react-hooks";
import { Form, Button, Row, Col, InputGroup } from "react-bootstrap";
import FormAlert from "../components/FormAlert";

const TargetPage = () => {
  const [target, setTarget] = useState({ x: "", y: "", speed: "" });
  const [show, setShow] = useState(false);
  const [errors, setErrors] = useState({});

  const { client } = useMqttState();

  const handleValidation = () => {
    let formIsValid = true;
    let error = {};
    let data = target;

    // target x
    if (!data["x"]) {
      formIsValid = false;
      error["x"] = "target x coordinate is empty!";
    } else if (isNaN(data["x"])) {
      formIsValid = false;
      error["x"] = "target x coordinate must be a number!";
    }

    // target y
    if (!data["y"]) {
      formIsValid = false;
      error["y"] = "target y coordinate is empty!";
    } else if (isNaN(data["y"])) {
      formIsValid = false;
      error["y"] = "target y coordinate must be a number!";
    }

    // Speed
    if (!typeof data["speed"] === "number") {
      formIsValid = false;
      error["speed"] = "Speed field must be a number!";
    } else if (data["speed"] < 0 || data["speed"] > 255) {
      formIsValid = false;
      error["speed"] = "Speed must be between 0 and 255";
    } else if (isNaN(data["speed"])) {
      formIsValid = false;
      error["speed"] = "Speed has to be a number!";
    }

    setErrors(error);
    return formIsValid;
  };

  const handleChange = (e) => {
    setTarget({ ...target, [e.target.name]: e.target.value });
  };

  const handleSubmit = (e) => {
    e.preventDefault();

    if (handleValidation()) {
      client.publish("drive/t2c", JSON.stringify(target));
      setTarget({ x: "", y: "", speed: "" });
    } else {
      setShow(true);
    }
  };

  return (
    <React.Fragment>
      <FormAlert
        show={show}
        setShow={setShow}
        home={false}
        errors={errors}
        setErrors={setErrors}
      />
      <Map />
      <Form onSubmit={handleSubmit} className="mx-4">
        <Row>
          <Col>
            <Form.Label>Target x-coordinate</Form.Label>
            <Form.Control
              name="x"
              size="text"
              value={target.x}
              placeholder="0"
              onChange={handleChange}
            />
          </Col>
          <Col>
            <Form.Label>Target y-coordinate</Form.Label>
            <Form.Control
              name="y"
              size="text"
              value={target.y}
              placeholder="0"
              onChange={handleChange}
            />
          </Col>
          <Col>
            <Form.Label>Speed</Form.Label>
            <InputGroup>
              <Form.Control
                name="speed"
                size="text"
                value={target.speed}
                placeholder="0"
                onChange={handleChange}
              />
              <InputGroup.Append>
                <InputGroup.Text>pwm</InputGroup.Text>
              </InputGroup.Append>
            </InputGroup>
          </Col>
        </Row>
        <br />
        <Button variant="light" type="submit">
          Submit
        </Button>
      </Form>
    </React.Fragment>
  );
};

export default TargetPage;
