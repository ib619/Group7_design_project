clear; close all;

import1 = readmatrix('BATCYCLE2.CSV');
import1 = import1(18:3427,:); % for BATCYCLE2.csv
length = size(import1, 1);

import2 = readmatrix('DIAGNOSE2.CSV');
import2 = import2(end-length:end, :);

dvimport = readmatrix('dv_SoC.csv');

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

dv1 = dvimport(:,1);
dv2 = dvimport(:,2);
dv3 = dvimport(:,3);
dSoC = dvimport(:,4);

SOC1 = zeros(length, 1);
SOC2 = zeros(length, 1);
SOC3 = zeros(length, 1);

d_ocv_l = 3150;
d_ocv_u = 3300;

%% 

correction = 2;

for i = 1:length
    if (i==1 || state(i,1) == 0) % calibrate initial SoC with voltage lookup
        [c1, index1] = min(abs(dv1-V_1(i,1)));
        SOC1(i,1) = dSoC(index1);
        
        [c2, index2] = min(abs(dv2-V_2(i,1)));
        SOC2(i,1) = dSoC(index2);
        
        [c3, index3] = min(abs(dv3-V_3(i,1)));
        SOC3(i,1) = dSoC(index3);
    elseif (state(i,1) == 3 && state(i-1,1) == 0) % First valid term
        [c1, index1] = min(abs(dv1-V_1(i,1)));
        SOC1(i,1) = dSoC(index1);
        
        [c2, index2] = min(abs(dv2-V_2(i,1)));
        SOC2(i,1) = dSoC(index2);
        
        [c3, index3] = min(abs(dv3-V_3(i,1)));
        SOC3(i,1) = dSoC(index3);
    elseif (state(i,1) == 5 || state(i,1) == 0) % freeze SOC
        SOC1(i,1) = SOC1(i-1,1); 
        SOC2(i,1) = SOC2(i-1,1); 
        SOC3(i,1) = SOC3(i-1,1); 
    elseif (state(i,1) == 3) % discharge
        if (V_1(i,1) < d_ocv_l || V_1(i,1) > d_ocv_u) % outside threshold for CC?
            [c1, index1] = min(abs(dv1-V_1(i,1)));
            movavg = (movmean([SOC1(1:i-1); dSoC(index1)], [10 0])); % moving avg
            SOC1(i,1) = movavg(end);
        else
            SOC1(i,1) = SOC1(i-1,1)- 0.25/1800 * 100*correction;
            % SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
        if (V_2(i,1) < d_ocv_l || V_2(i,1) > d_ocv_u) % outside threshold for CC?
            [c2, index2] = min(abs(dv2-V_2(i,1)));
            movavg = (movmean([SOC2(1:i-1); dSoC(index2)], [10 0])); % moving avg
            SOC2(i,1) = movavg(end);
        else
            SOC2(i,1) = SOC2(i-1,1)- 0.25/1800 * 100*correction;
            % SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
        if (V_3(i,1) < d_ocv_l || V_3(i,1) > d_ocv_u) % outside threshold for CC?
            [c3, index3] = min(abs(dv3-V_3(i,1)));
            movavg = (movmean([SOC3(1:i-1); dSoC(index3)], [10 0])); % moving avg
            SOC3(i,1) = movavg(end);
        else
            SOC3(i,1) = SOC3(i-1,1)- 0.25/1800 * 100*correction;
            % SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
    end
end

remaining_time = zeros(length,1);
for i = 1:length
    % gross_Charge = (SOC1(i,1)+SOC2(i,1)+SOC3(i,1))*1800/100;
    amphr_cap = (min([SOC1(i,1) SOC2(i,1) SOC3(i,1)]))*1800/100;
    % remaining_Ws = (V_1(i,1)+V_2(i,1)+V_3(i,1)-3*2500)/1000*amphr_cap;
    remaining_Ws = (3*(min([V_1(i,1) V_2(i,1) V_3(i,1)])-2400))/1000*amphr_cap;
    remaining_time(i,1) = remaining_Ws/2 * 1.5;
end

%% 

figure(1);

subplot(3,1,1);
plot(V_1,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(V_2,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(V_3,'Linewidth', 2, 'MarkerSize', 8);
ylabel('Voltage (mV)');
xlabel('Time (s)');
legend('V_1','V_2','V_3','Location','Best');
ylim([2400 3700]);
set(gca,'LineWidth',2);
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(3,1,2);
plot(current_ref,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(current_measure,'Linewidth', 2, 'MarkerSize', 8);
ylabel('Current (mA)');
xlabel('Time (s)');
legend('Reference Current','Measured Current','Location','Best');
set(gca,'LineWidth',2);
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(3,1,3);
plot(disc1,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(disc2,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(disc3,'Linewidth', 2, 'MarkerSize', 8); hold on;
ylabel('Discharge ON/OFF');
xlabel('Time (s)');
legend('Cell 1 Discharge','Cell 2 Discharge','Cell 3 Discharge','Location','Best');
set(gca,'LineWidth',2);
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')
sgtitle('Series Cells Discharging Characteristic','FontName', 'Arial');

figure(2);
plot(SOC1,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(SOC2,'Linewidth', 2, 'MarkerSize', 8); hold on;
plot(SOC3,'Linewidth', 2, 'MarkerSize', 8);
ylabel('SOC (%)');
xlabel('Time (s)');
legend('SOC1','SOC2','SOC3','Location','Best');
ylim([0 110]);
set(gca,'LineWidth',2);
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')
sgtitle('Series Cells Discharging SoC','FontName', 'Arial');

figure(3);
plot(remaining_time);
xlabel('Time(s)');
ylabel('Remaining Time (s)');
