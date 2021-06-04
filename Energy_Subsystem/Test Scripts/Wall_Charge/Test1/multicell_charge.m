clear; close all;

import1 = readmatrix('BATCYCLE.CSV');
import1 = import1(368:end, :);
length = size(import1, 1);

import2 = readmatrix('DIAGNOSE.CSV');
import2 = import2(end-length:end, :);

cvimport = readmatrix('cv_SoC.csv');

%% 

state = import1(:,1);
V_1 = import1(:,2);
V_2 = import1(:,3);
V_3 = import1(:,4);

current_ref = import1(:,5);
current_measure = import1(:,6);

disc1 = import1(:,7);
disc2 = import1(:,8);
disc3 = import1(:,9);

% SOC1 = import2(:,5);
% SOC2 = import2(:,6);
% SOC3 = import2(:,7);

cv1 = cvimport(:,1);
cv2 = cvimport(:,2);
cv3 = cvimport(:,3);
cSoC = cvimport(:,4);

SOC1 = zeros(length, 1);
SOC2 = zeros(length, 1);
SOC3 = zeros(length, 1);

c_ocv_l = 3300;
c_ocv_u = 3450;

for i = 1:length
    if (state(i,1) == 0) % calibrate initial SoC with voltage lookup
        [c1, index1] = min(abs(cv1-V_1(i,1)));
        SOC1(i,1) = cSoC(index1);
        
        [c2, index2] = min(abs(cv2-V_2(i,1)));
        SOC2(i,1) = cSoC(index2);
        
        [c3, index3] = min(abs(cv3-V_3(i,1)));
        SOC3(i,1) = cSoC(index3);
    elseif (state(i,1) == 1 && state(i-1,1) == 0) % First valid term
        [c1, index1] = min(abs(cv1-V_1(i,1)));
        SOC1(i,1) = cSoC(index1);
        
        [c2, index2] = min(abs(cv2-V_2(i,1)));
        SOC2(i,1) = cSoC(index2);
        
        [c3, index3] = min(abs(cv3-V_3(i,1)));
        SOC3(i,1) = cSoC(index3);
    elseif (state(i,1) == 5 || state(i,1) == 0) % freeze SOC
        SOC1(i,1) = SOC1(i-1,1); 
        SOC2(i,1) = SOC2(i-1,1); 
        SOC3(i,1) = SOC3(i-1,1); 
    elseif (state(i,1) == 1 || state(i,1) == 6) % charge
        if (V_1(i,1) < c_ocv_l || V_1(i,1) > c_ocv_u) % outside threshold for CC?
            [c1, index1] = min(abs(cv1-V_1(i,1)));
            movavg = (movmean([SOC1(1:i-1); cSoC(index1)], [10 0])); % moving avg
            SOC1(i,1) = movavg(end);
        else
            SOC1(i,1) = SOC1(i-1,1)+ 0.25/1800 * 100;
            % SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
        if (V_2(i,1) < c_ocv_l || V_2(i,1) > c_ocv_u) % outside threshold for CC?
            [c2, index2] = min(abs(cv2-V_2(i,1)));
            movavg = (movmean([SOC2(1:i-1); cSoC(index2)], [10 0])); % moving avg
            SOC2(i,1) = movavg(end);
        else
            SOC2(i,1) = SOC2(i-1,1)+ 0.25/1800 * 100;
            % SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
        if (V_3(i,1) < c_ocv_l || V_3(i,1) > c_ocv_u) % outside threshold for CC?
            [c3, index3] = min(abs(cv3-V_3(i,1)));
            movavg = (movmean([SOC3(1:i-1); cSoC(index3)], [10 0])); % moving avg
            SOC3(i,1) = movavg(end);
        else
            SOC3(i,1) = SOC3(i-1,1)+ 0.25/1800 * 100;
            % SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
    end
end

%% 
figure(1);

subplot(3,1,1);
plot(V_1); hold on;
plot(V_2); hold on;
plot(V_3);
ylabel('Voltage (mV)');
xlabel('Time (s)');
legend('V_1','V_2','V_3','location','best');

subplot(3,1,2);
plot(current_ref); hold on;
plot(current_measure);
ylabel('Current (mA)');
xlabel('Time (s)');
legend('Reference Current','Measured Current','location','best');

subplot(3,1,3);
plot(SOC1); hold on;
plot(SOC2); hold on;
plot(SOC3);
ylabel('SOC (%)');
xlabel('Time (s)');
legend('SOC1','SOC2','SOC3','location','best');