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

  const { client } = useMqttState();

  const handleChange = (e) => {
    setTarget({ ...target, [e.target.name]: e.target.value });
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    if (
      target.direction === "" ||
      target.speed === "" ||
      target.distance === ""
    ) {
      console.log("No target coordinate set");
    } else {
      client.publish("drive/discrete", JSON.stringify(target));
      setShow(true);
      console.log(target);
    }
    setTarget({ direction: "", speed: "", distance: "" });
  };

  return (
    <>
      <FormAlert show={show} setShow={setShow} />
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
                <InputGroup.Text>cm/s</InputGroup.Text>
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
                <InputGroup.Text>cm</InputGroup.Text>
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
