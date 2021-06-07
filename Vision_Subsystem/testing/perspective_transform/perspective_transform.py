import cv2
import numpy as np
from rig.type_casts import float_to_fp, fp_to_float

pts1 = np.float32([[275, 310], [390, 310], [640, 480], [0, 480]])
# pts1 = np.float32([[250, 240], [390, 240], [640, 480], [0, 480]])
pts2 = np.float32([[0, 0],[640, 0],[640, 480],[0, 480]])

M = cv2.getPerspectiveTransform(pts1, pts2)

def get_fixed_point():
    ftp = float_to_fp(signed=True, n_bits=24, n_frac=12)
    print("M1:", M[0][0], "Fixed Point(absolute):", hex(int(ftp(abs(M[0][0])))))
    print("M2:", M[0][1], "Fixed Point(absolute):", hex(int(ftp(abs(M[0][1])))))
    print("M3:", M[0][2], "Fixed Point(absolute):", hex(int(ftp(abs(M[0][2])))))

    print("M4:", M[1][0], "Fixed Point(absolute):", hex(int(ftp(abs(M[1][0])))))
    print("M5:", M[1][1], "Fixed Point(absolute):", hex(int(ftp(abs(M[1][1])))))
    print("M6:", M[1][2], "Fixed Point(absolute):", hex(int(ftp(abs(M[1][2])))))

    print("M7:", M[2][0], "Fixed Point(absolute):", hex(int(ftp(abs(M[2][0])))))
    print("M8:", M[2][1], "Fixed Point(absolute):", hex(int(ftp(abs(M[2][1])))))
    print("M9:", M[2][2], "Fixed Point(absolute):", hex(int(ftp(abs(M[2][2])))))

def image_perspective(image_path = '../images/perspective.jpg'):
    frame = cv2.imread(image_path)
    frame = cv2.resize(frame, (640, 480))
    x1, y1 = 251, 376
    x2, y2 = 192, 396
    x3, y3 = 250, 375
    x4, y4 = 316, 237
    cv2.circle(frame,(x1,y1),4,(0, 0, 255), 3)    
    cv2.circle(frame,(x2,y2),4,(0, 0, 255), 3)    
    cv2.circle(frame,(x3,y3),4,(0, 0, 255), 3)    
    cv2.circle(frame,(x4,y4),4,(0, 0, 255), 3)    
    print(M, type(M))
    int_array = M. astype(int)
    print(int_array, type(int_array))

    circle_x1 = int((M[0][0]*x1 + M[0][1]*y1 + M[0][2])/(M[2][1] * y1 + 1))
    circle_y1 = int((M[1][1]*y1 + M[1][2])/(M[2][1] * y1 + 1))
    print(circle_x1, circle_y1)
    circle_x2 = int((M[0][0]*x2 + M[0][1]*y2 + M[0][2])/(M[2][1] * y2+ 1))
    circle_y2 = int((M[1][1]*y2 + M[1][2])/(M[2][1] * y2 + 1))
    print(circle_x2, circle_y2)
    circle_x3 = int((M[0][0]*x3 + M[0][1]*y3 + M[0][2])/(M[2][1] * y3+ 1))
    circle_y3 = int((M[1][1]*y3 + M[1][2])/(M[2][1] * y3 + 1))
    print(circle_x3, circle_y3)
    circle_x4 = int((M[0][0]*x4 + M[0][1]*y4 + M[0][2])/(M[2][1] * y4+ 1))
    circle_y4 = int((M[1][1]*y4 + M[1][2])/(M[2][1] * y4 + 1))
    print(circle_x4, circle_y4)
    while True:
        result = cv2.warpPerspective(frame, M, (640, 480))
        cv2.circle(result,(circle_x1,circle_y1),4,(0, 255, 255), 3)    
        cv2.circle(result,(circle_x2,circle_y2),4,(0, 255, 255), 3)
        cv2.circle(result,(circle_x3,circle_y3),4,(0, 255, 255), 3)
        cv2.circle(result,(circle_x4,circle_y4),4,(0, 255, 255), 3)
        cv2.circle(result,(276,116),4,(0, 0, 255), 3) 
        cv2.circle(result,(499,116),4,(0, 0, 255), 3)
        cv2.circle(result,(525,253),4,(0, 0, 255), 3) 
        cv2.circle(result,(387,464),4,(0, 0, 255), 3) 
        cv2.imshow('frame', frame) # Inital Capture
        cv2.imshow('frame1', result) # Transformed Capture
    
        if cv2.waitKey(24) == 27:
            break

    cv2.destroyAllWindows()


def live_perspective():
    cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
   # compute the perspective transform matrix and then apply it

    while True:
        ret, frame = cap.read()
        result = cv2.warpPerspective(frame, M, (640, 480))
        cv2.imshow('frame', frame) # Inital Capture
        cv2.imshow('frame1', result) # Transformed Capture
    
        if cv2.waitKey(24) == 27:
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    live_perspective()
    # image_perspective()
    # get_fixed_point()
