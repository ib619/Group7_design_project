import React, { useState } from "react";
import Map from "../components/Map";
import { useMqttState } from "mqtt-react-hooks";
import { Form, Button, Row, Col } from "react-bootstrap";

const TargetPage = () => {
  const [target, setTarget] = useState({ x: "", y: "" });
  const { client } = useMqttState();

  const handleChange = (e) => {
    setTarget({ ...target, [e.target.name]: e.target.value });
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    if (target.x === "" || target.y === "") {
      console.log("No target coordinate set");
    } else {
      client.publish("drive/t2c", JSON.stringify(target));
      console.log(target);
    }
    setTarget({ x: "", y: "" });
  };

  return (
    <>
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
        </Row>
        <br />
        <Button type="submit">Submit</Button>
      </Form>
    </>
  );
};

export default TargetPage;
