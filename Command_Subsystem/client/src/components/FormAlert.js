import React from "react";
import { Alert, Button } from "react-bootstrap";

const FormAlert = (props) => {
  return (
    <Alert show={props.show} variant="success">
      <Alert.Heading>Success!!</Alert.Heading>
      <p>Your commands are being sent to the rover :></p>
      <hr />
      <div className="d-flex justify-content-end">
        <Button onClick={() => props.setShow(false)} variant="outline-success">
          Ok bye!
        </Button>
      </div>
    </Alert>
  );
};

export default FormAlert;
