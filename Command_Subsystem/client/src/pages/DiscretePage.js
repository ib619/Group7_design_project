import React, { useState } from "react";
import { useMqttState } from "mqtt-react-hooks";
import { Form, Button, Row, Col, InputGroup } from "react-bootstrap";
import Map from "../components/Map";
import FormAlert from "../components/FormAlert";

const DiscretePage = (props) => {
  // for the game
  const [show, setShow] = useState(false);
  const [target, setTarget] = useState({
    direction: "",
    speed: "",
    distance: "",
  });
  const [valid, setValid] = useState(false);
  const [errors, setErrors] = useState({});

  const { client } = useMqttState();

  const handleValidation = () => {
    let formIsValid = true;
    let error = {};
    let data = target;

    // Distance
    if (!data["distance"]) {
      formIsValid = false;
      error["distance"] = "Distance field is empty!";
    } else if (data["distance"] === 0) {
      formIsValid = false;
      error["distance"] = "Distance cannot be 0!";
    }

    // Speed
    if (!data["speed"]) {
      formIsValid = false;
      error["speed"] = "Speed field is empty!";
    } else if (data["speed"] < 0 || data["speed"] > 255) {
      formIsValid = false;
      error["speed"] = "Speed must be between 0 and 255";
    }

    // Direction
    if (!data["direction"]) {
      formIsValid = false;
      error["direction"] = "Direction field is empty!";
    } else if (data["direction"] < -180 || data["direction"] > 180) {
      formIsValid = false;
      error["direction"] = "Direction must be between -180˚ and 180˚";
    }

    setErrors(error);
    setValid(formIsValid);
    return formIsValid;
  };

  const handleChange = (e) => {
    setTarget({ ...target, [e.target.name]: e.target.value });
  };

  // TODO: error checking for values
  const handleSubmit = (e) => {
    e.preventDefault();

    // if valid
    if (handleValidation()) {
      client.publish("drive/discrete", JSON.stringify(target));
    }

    // distance: any not equal 0
    // speed: 0 to 255
    // angle: -180 to 180
    setTarget({ direction: "", speed: "", distance: "" });
    setShow(true);
  };

  return (
    <>
      <FormAlert
        show={show}
        setShow={setShow}
        valid={valid}
        errors={errors}
        setErrors={setErrors}
      />
      <Map pos={props.pos} />
      <Form onSubmit={handleSubmit} className="mx-4">
        <Row>
          <Col>
            <Form.Label>Direction</Form.Label>
            <InputGroup>
              <Form.Control
                name="direction"
                size="text"
                value={target.direction}
                placeholder="0"
                onChange={handleChange}
              />
              <InputGroup.Append>
                <InputGroup.Text>˚</InputGroup.Text>
              </InputGroup.Append>
            </InputGroup>
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
          <Col>
            <Form.Label>Distance</Form.Label>
            <InputGroup>
              <Form.Control
                name="distance"
                size="text"
                value={target.distance}
                placeholder="0"
                onChange={handleChange}
              />
              <InputGroup.Append>
                <InputGroup.Text>mm</InputGroup.Text>
              </InputGroup.Append>
            </InputGroup>
          </Col>
        </Row>
        <br />
        <Button type="submit">Submit</Button>
      </Form>
    </>
  );
};

export default DiscretePage;
