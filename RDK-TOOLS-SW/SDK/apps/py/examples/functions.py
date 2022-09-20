from dis import dis
from math import dist
import numpy as np

sigma_r, p0, sigma_noise = 0.01, 2, 0.1
R = sigma_r*np.eye(2)
P0= p0*np.eye(4)
dt = 0.1
Q = sigma_noise*np.eye(4)
H = np.array([[1,0,0,0],[0,1,0,0]])

class kalman():
    def __init__(self, dt: float, z0 = None, v0 = None):
        self.x = np.array([z0, v0]).reshape(-1,)
        self.F = np.array([[1,0,dt,0], [0,1,0,dt], [0,0,1,0], [0,0,0,1]])
        self.P = P0
        self.K = None
        self.P_= self.P
        self.x_= self.x
        self.m = z0
        self.dt = dt
        
    def predict(self):
        self.x = self.F.dot(self.x_)
        self.P = self.F.dot(self.P_).dot(self.F.T)+Q
        
    def update(self):
        self.K = self.P@(H.T)@np.linalg.inv(H@self.P@H.T+R)
        self.x_ = self.x + self.K@(self.m - H@self.x)
        self.P_ = (np.eye(4)-self.K@H)@self.P
        return self.x_

def polardistance(a,b):
        return np.sqrt(a[0]**2+b[0]**2-a[0]*b[0]*np.cos(a[1]-b[1]))

class filter_noise():
    def __init__(self, threshold = 0.97, max_dis = 0.45) -> None:
        self.threshold = threshold
        self.gating = max_dis

    def find_peaks(self, data, prev, r, t, scale):
        gated_peaks = np.zeros(data.shape)
        peaks = ((data/scale)>self.threshold-1)
        for angle_idx in range(data.shape[1]):
            for range_idx in range(data.shape[0]):
                if peaks[range_idx, angle_idx]:
                    if polardistance(prev, [r[range_idx], (t[angle_idx]+90)*np.pi/180])<self.gating:
                        gated_peaks[range_idx, angle_idx] = data[range_idx, angle_idx]
        r_idx, a_idx =  np.unravel_index(gated_peaks.argmin(), gated_peaks.shape)
        return r[r_idx], t[a_idx]