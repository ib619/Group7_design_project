close all; clear;

% Number of entries
% Charge 1: 7028
% Discharge 1: 7140
% Charge 2: 7899
% Discharge 2: 7970
% Charge 3: 7612
% Discharge 3: 7655

%% 
charge_1 = load('charge_1.mat');
charge_2 = load('charge_2.mat');
charge_3 = load('charge_3.mat');
discharge_1 = load('discharge_1.mat');
discharge_2 = load('discharge_2.mat');
discharge_3 = load('discharge_3.mat');

%%

charge_1_mapped = zeros(105,1);
j = 1;
for i = 1:size(charge_1.charge_table)
    if (i == 1 || mod(i,70) == 0)
        charge_1_mapped(j) = charge_1.charge_table(i);
        j = j+1;
    end
end


charge_2_mapped = zeros(105,1);
j = 1;
for i = 1:size(charge_2.charge_table)
    if (i == 1 || mod(i,78) == 0)
        charge_2_mapped(j) = charge_2.charge_table(i);
        j = j+1;
    end
end

charge_3_mapped = zeros(105,1);
j = 1;
for i = 1:size(charge_3.charge_table)
    if (i == 1 || mod(i,76) == 0)
        charge_3_mapped(j) = charge_3.charge_table(i);
        j = j+1;
    end
end

charge_SoC = linspace(0,100,100);
charge_SoC = charge_SoC';
charge_1_mapped = charge_1_mapped(1:100);
charge_2_mapped = charge_2_mapped(1:100);
charge_3_mapped = charge_3_mapped(1:100);
cv_SoC = [charge_1_mapped charge_2_mapped charge_3_mapped charge_SoC];
writematrix(cv_SoC,'cv_SoC.csv') 

%%

discharge_1_mapped = zeros(105,1);
j = 1;
for i = 1:size(discharge_1.discharge_table)
    if (i == 1 || mod(i,71) == 0)
        discharge_1_mapped(j) = discharge_1.discharge_table(i);
        j = j+1;
    end
end

discharge_2_mapped = zeros(105,1);
j = 1;
for i = 1:size(discharge_2.discharge_table)
    if (i == 1 || mod(i,76) == 0)
        discharge_2_mapped(j) = discharge_2.discharge_table(i);
        j = j+1;
    end
end

discharge_3_mapped = zeros(105,1);
j = 1;
for i = 1:size(discharge_3.discharge_table)
    if (i == 1 || mod(i,76) == 0)
        discharge_3_mapped(j) = discharge_3.discharge_table(i);
        j = j+1;
    end
end

discharge_SoC = linspace(100,0,100);
discharge_SoC = discharge_SoC';
discharge_1_mapped = discharge_1_mapped(1:100);
discharge_2_mapped = discharge_2_mapped(1:100);
discharge_3_mapped = discharge_3_mapped(1:100);
dv_SoC = [discharge_1_mapped discharge_2_mapped discharge_3_mapped discharge_SoC];
writematrix(dv_SoC,'dv_SoC.csv') 



