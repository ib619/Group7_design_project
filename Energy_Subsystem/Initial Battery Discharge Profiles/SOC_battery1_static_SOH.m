clear; close all;

import = readmatrix('BATCYCLE3.CSV');

% Arbitrary decisions
    % Charge: Calibrate SoC with voltage lookup at above 3400V;
    % Discharge: Calibrate SoC with voltage Lookup at below 3200V;
    
% Overall algorithm
    % Determine upper and lower thresholds for OCV estimation, with values
    % initialised at
        % d_ocv_l = 3100;
        % d_ocv_u = 3200;
        % c_ocv_u = 3400;
        % c_ocv_l = 3300;
    % Coulomb counting in between thresholds, OCV below, above thresholds

%% Seperate Plots
state = import(:,1);
voltage = import(:,2);
current_ref = import(:,3);
current = import(:,4);
length = size(import, 1);

% Indices indicating start and end of discharge cycles
ds = 0; % discharge start
de = 0; % discharge end
cs = 0; % charge start
ce = 0; % charge end
discharged = 0;

d_ocv_l = 3100;
d_ocv_u = 3200;
c_ocv_u = 3400;
c_ocv_l = 3300;

actual_charge = 0;

% Identify start and end of discharge, as well as thresholds for OCV
% Also track actual charge
for i = 1:length
    if(state(i,1) == 3)
        actual_charge = actual_charge + 0.25;
    end
    if(state(i,1) == 3 && state(i-1) == 2)
       ds = i;
    end
    if (state(i,1) == 4 && state(i-1) == 3)
        de = i;
        discharged = 1;
    end
    if (state(i,1) == 1  && state(i-1) == 4 && discharged == 1)
        cs = i;
    end
    % Note: define state 6 for constant voltage charging (after state 1)
    if (state(i,1) == 2  && (state(i-1) == 1 || state(i-1) == 6) && discharged == 1)
        ce = i;
        break;
    end
end

discharge_time = de-ds+1;
charge_time = ce-cs+1;
    
discharge_table = import(ds:de,2);
discharge_SOC = (linspace(1,0,discharge_time))';
discharge_SOC_MA = movmean(discharge_SOC, [100 0]);
discharge_MA = movmean(discharge_table, [300 0]); % moving average
dv_dt = diff(discharge_MA);
discharge_map = [discharge_table, discharge_SOC, discharge_MA, [0;dv_dt]];

charge_table = import(cs:ce,2);
charge_SOC = (linspace(0,1,charge_time))';
charge_SOC_MA = movmean(charge_SOC, [100 0]);
charge_MA = movmean(charge_table, [300 0]); % moving average
dv_ct = diff(charge_MA);
charge_map = [charge_table, charge_SOC, charge_MA, [0;dv_ct]];

% Identify thresholds for OCV
for i = 1:size(dv_dt,1) % upper discharge OCV
    if(dv_dt(i,1) > -0.12 && discharge_MA(i,1) > 3200)
        d_ocv_u = discharge_MA(i,1);
        break;
    end
end
for i = 1:size(dv_dt,1) % lower discharge OCV
    if(dv_dt(i,1) < -0.08 && discharge_MA(i,1) < 3100)
        d_ocv_l = discharge_MA(i,1);
        break;
    end
end
for i = 1:size(dv_ct,1) % lower charge OCV
    if(dv_ct(i,1) < 0.12 && charge_MA(i,1) < 3300)
        c_ocv_l = charge_MA(i,1);
        break;
    end
end
for i = 1:size(dv_ct,1) % upper charge OCV
    if(dv_ct(i,1) > 0.1 && charge_MA(i,1) > 3400)
        c_ocv_u = charge_MA(i,1);
        break;
    end
end


%% Plot V vs SoC curves

figure(1);
plot (charge_SOC*100, charge_table, 'r.','Linewidth', 2, 'MarkerSize', 8); hold on;
plot (charge_SOC*100, charge_MA, 'r-','Linewidth', 2, 'MarkerSize', 8); hold on;
plot (discharge_SOC*100, discharge_table, 'b.','Linewidth', 2, 'MarkerSize', 8); hold on;
plot (discharge_SOC*100, discharge_MA, 'b-','Linewidth', 2, 'MarkerSize', 8); hold on;
xlabel('SOC (%)');
ylabel('Terminal Voltage (mV)');
legend('Charge','Discharge');
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')
% title('Charging and Discharging Curves');

set(gca, 'FontName', 'Arial')
sgtitle('Charging and Discharging Curve','FontName', 'Arial');

%% Iterate through and determine total charge. Apply this to the first iteration as well.

SOC = zeros(length,1);
SOC_cc = zeros(length,1);
nominal_charge = 1876;

%% This is assuming full charge and discharge 

% To ensure that charge is correct for first discharge

for i = 1:length
    if (i == 1)
        SOC(i,1) = 0;
    elseif (state(i,1) == 1 && state(i-1,1) == 0) % First valid term
        [c, index] = min(abs(charge_table-voltage(i,1)));
        c0 = charge_SOC_MA(index);
        SOC(i,1) = c0;
    elseif (state(i,1) == 5 || state(i,1) == 0) % freeze SOC
        SOC(i,1) = SOC(i-1,1); 
    elseif (state(i-1,1) == 0) % calibrate initial SoC with voltage lookup
        [c, index] = min(abs(charge_table-voltage(i,1)));
        SOC(i,1) = charge_SOC_MA(index);
    elseif (state(i,1) == 1 || state(i,1) == 6) % charge
        if (voltage(i,1) < c_ocv_l) % below threshold for OCV?
            [c, index] = min(abs(charge_table-voltage(i,1)));
            SOC(i,1) = charge_SOC_MA(index);
        elseif (voltage(i,1) > c_ocv_u) % reach upper threshold for OCV?
            [c, index] = min(abs(charge_table-voltage(i,1)));
            movavg = (movmean([SOC(1:i-1); charge_SOC_MA(index)], [10 0])); % moving avg
            SOC(i,1) = movavg(end);
        else
            SOC(i,1) = SOC(i-1,1)+ 0.25/nominal_charge;
            SOC_cc(i,1) = SOC(i,1); % track the coulomb counting values
        end
    elseif (state(i,1) == 2) % charge rest
        SOC(i,1) = 1;
    elseif (state(i,1) == 3) % discharge
        if (voltage(i,1) < d_ocv_l || voltage(i,1) > d_ocv_u || SOC(i-1,1) < 0.2)
            [c, index] = min(abs(discharge_table-voltage(i,1)));
            movavg = (movmean([SOC(1:i-1); discharge_SOC_MA(index)], [10 0])); % moving avg
            SOC(i,1) = movavg(end);
        else
            SOC(i,1) = SOC(i-1,1) - 0.25/nominal_charge;
            SOC_cc(i,1) = SOC(i,1);  % track the coulomb counting values
        end      
    elseif (state(i,1) == 4) % discharge rest
        SOC(i,1) = 0;
    end
end

%% Plots
% each time interval is 1s. convenient!

figure(2);
% subplot(4,1,1);
% plot(state,'Linewidth', 2, 'MarkerSize', 8);
% xlabel('Time(s)');
% ylabel('State');
% title('Battery Characteristic');
% set(gca,'LineWidth',2)
% set(gca,'FontSize',12)
% set(gca, 'FontName', 'Arial')

subplot(3,1,1);
plot(voltage,'Linewidth', 2, 'MarkerSize', 8);
xlabel('Time(s)');
ylabel('Voltage (mV)');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(3,1,2);
plot(current_ref, 'r','Linewidth', 2, 'MarkerSize', 8); hold on;
plot(current,'b','Linewidth', 2, 'MarkerSize', 8);
xlabel('Time(s)');
ylabel('Current (mA)');
legend('Reference', 'Actual','Location','best');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(3,1,3);
plot(SOC*100,'Linewidth', 3, 'MarkerSize', 8); hold on;
plot(SOC_cc*100,'Linewidth', 2, 'MarkerSize', 8);
legend('SOC', 'SOC Coulomb Count','Location','best');
xlabel('Time(s)');
ylabel('SOC (%)');
ylim([-20 120]);
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

set(gca, 'FontName', 'Arial')
sgtitle('Battery Characteristic','FontName', 'Arial');
