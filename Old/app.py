import sys
import serial
import numpy as np
import pyqtgraph.opengl as gl
from PyQt5.QtWidgets import *
from PyQt5 import QtCore, QtGui
from PyQt5.QtGui import *
from PyQt5.QtCore import *
import sys

# --- Config UART ---
ser = serial.Serial("COM3", 115200, timeout=1)  # adapte COM port

# --- Fenêtre OpenGL ---
app = QApplication(sys.argv)
w = gl.GLViewWidget()
w.show()
w.setWindowTitle("IMU Orientation Viewer")
w.setCameraPosition(distance=10)

# Grille 3D
g = gl.GLGridItem()
g.scale(2,2,1)
w.addItem(g)

# Cube (boîte)
verts = np.array([
    [1,1,1], [1,1,-1], [1,-1,-1], [1,-1,1],
    [-1,1,1], [-1,1,-1], [-1,-1,-1], [-1,-1,1]
])
faces = np.array([
    [0,1,2], [0,2,3],
    [4,5,6], [4,6,7],
    [0,1,5], [0,5,4],
    [2,3,7], [2,7,6],
    [1,2,6], [1,6,5],
    [0,3,7], [0,7,4]
])
colors = np.array([[1,0,0,0.8] for _ in faces])  # rouge semi-transp
cube = gl.GLMeshItem(vertexes=verts, faces=faces, faceColors=colors, smooth=False, drawEdges=True)
cube.scale(1,1,1)
w.addItem(cube)

# --- Rotation utils ---
def euler_to_matrix(roll, pitch, yaw):
    # roll=X, pitch=Y, yaw=Z (en degrés)
    r, p, y = np.radians([roll, pitch, yaw])

    Rx = np.array([[1,0,0,0],
                   [0,np.cos(r),-np.sin(r),0],
                   [0,np.sin(r), np.cos(r),0],
                   [0,0,0,1]])

    Ry = np.array([[ np.cos(p),0,np.sin(p),0],
                   [0,1,0,0],
                   [-np.sin(p),0,np.cos(p),0],
                   [0,0,0,1]])

    Rz = np.array([[np.cos(y),-np.sin(y),0,0],
                   [np.sin(y), np.cos(y),0,0],
                   [0,0,1,0],
                   [0,0,0,1]])

    return Rz @ Ry @ Rx


def quat_to_matrix(qw, qx, qy, qz):
    # normaliser
    n = np.sqrt(qw*qw + qx*qx + qy*qy + qz*qz)
    qw, qx, qy, qz = qw/n, qx/n, qy/n, qz/n
    
    # matrice de rotation 4x4
    R = np.array([
        [1-2*(qy*qy+qz*qz),   2*(qx*qy - qz*qw),   2*(qx*qz+qy*qw), 0],
        [2*(qx*qy+qz*qw),   1-2*(qx*qx+qz*qz),   2*(qy*qz - qx*qw), 0],
        [2*(qx*qz - qy*qw),   2*(qy*qz+qx*qw),   1-2*(qx*qx+qy*qy), 0],
        [0,0,0,1]
    ], dtype=np.float32)
    return R

# --- Timer update ---
def updateEuler():
    line = ser.readline().decode().strip()
    if line:
        try:
            roll, pitch, yaw = map(float, line.split(","))
            M = euler_to_matrix(roll, pitch, yaw)
            cube.resetTransform()
            cube.setTransform(QtGui.QMatrix4x4(*M.T.flatten()))  # Qt attend row-major
        except:
            pass

def updateQat():
    line = ser.readline().decode().strip()
    if line:
        try:
            qw, qx, qy, qz = map(float, line.split(","))
            M = quat_to_matrix(qw, qx, qy, qz)
            cube.resetTransform()
            cube.setTransform(QtGui.QMatrix4x4(*M.T.flatten()))
        except:
            pass

timer = QtCore.QTimer()
timer.timeout.connect(updateQat)
timer.start(20)

# --- Run ---
if __name__ == '__main__':
    QtGui.QGuiApplication.instance().exec_()
