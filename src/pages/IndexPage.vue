<template>
  <q-page padding class="q-pa-md">
    <q-layout view="lHh Lpr lFf" class="bg-grey-1">
      <q-page-container>
        <q-page padding class="row justify-center">
          <div class="col-12 col-md-7 col-lg-6 q-mb-md">
            <q-card flat bordered class="q-px-md">
              <q-card-section>
                <h6 class="text-weight-bold q-mb-md">心率 & 血氧 实时折线图</h6>
                <div ref="lineChart" style="width: 100%; height: 280px"></div>
              </q-card-section>
            </q-card>
            <q-card flat bordered class="q-px-md q-mt-md">
              <q-card-section>
                <h6 class="text-weight-bold q-mb-md">血氧 实时柱状图</h6>
                <div ref="barChart" style="width: 100%; height: 350px"></div>
              </q-card-section>
            </q-card>
          </div>

          <div class="col-12 col-md-5 col-lg-4">
            <!-- 心率统计卡 + 历史数据表分页 -->
            <q-card
              flat
              bordered
              :class="$q.screen.gt.sm ? 'q-mx-md' : ''"
              class="q-mb-md"
            >
              <q-card-section>
                <h6 class="text-weight-bold q-mb-md">心率数据统计</h6>
                <div class="row q-col-gutter-md">
                  <div class="col">
                    <p class="text-subtitle2 q-my-xs">心率 (bpm)</p>
                    <p>
                      最大值: <strong>{{ stats.HR.max }}</strong>
                    </p>
                    <p>
                      最小值: <strong>{{ stats.HR.min }}</strong>
                    </p>
                    <p>
                      平均值: <strong>{{ stats.HR.avg }}</strong>
                    </p>
                  </div>
                  <div class="col">
                    <div class="row justify-evenly">
                      <p>心率过低提醒</p>
                      <p>{{ stats.hrLowConter }}次</p>
                    </div>
                    <div class="row justify-evenly">
                      <p>心率过高提醒</p>
                      <p>{{ stats.hrHighConter }}次</p>
                    </div>
                  </div>
                </div>

                <q-separator class="q-my-md" />

                <h6 class="text-weight-bold q-mb-md">心率历史数据</h6>
                <q-table
                  :columns="heartRateColumns"
                  :rows="paginatedHRData"
                  flat
                  dense
                  v-model:pagination="hrPagination"
                  :rows-per-page-options="[5, 10, 15, 30, 50, 100]"
                />
              </q-card-section>
            </q-card>

            <!-- 血氧统计卡 + 历史数据表分页 -->
            <q-card
              flat
              bordered
              class="q-mt-md"
              :class="$q.screen.gt.sm ? 'q-mx-md' : ''"
            >
              <q-card-section>
                <h6 class="text-weight-bold q-mb-md">血氧数据统计</h6>
                <div class="row q-col-gutter-md">
                  <div>
                    <q-separator spaced />
                    <p class="text-subtitle2 q-my-xs">血氧 (%)</p>
                    <p>
                      最大值: <strong>{{ stats.SpO2.max }}</strong>
                    </p>
                    <p>
                      最小值: <strong>{{ stats.SpO2.min }}</strong>
                    </p>
                    <p>
                      平均值: <strong>{{ stats.SpO2.avg }}</strong>
                    </p>
                  </div>
                </div>

                <q-separator class="q-my-md" />

                <h6 class="text-weight-bold q-mb-md">血氧历史数据</h6>
                <q-table
                  :columns="spo2Columns"
                  :rows="paginatedSpO2Data"
                  flat
                  dense
                  v-model:pagination="spo2Pagination"
                  :rows-per-page-options="[5, 10, 15, 30, 50, 100]"
                />
              </q-card-section>
            </q-card>
          </div>
        </q-page>
      </q-page-container>
    </q-layout>
  </q-page>
</template>

<script setup>
import { ref, onMounted, onBeforeUnmount, reactive, computed } from "vue";
import * as echarts from "echarts";
import mqtt from "mqtt";
// mqtt相关
let client = null;
const mqttHost = "wss://ec1ffed0.ala.cn-hangzhou.emqxsl.cn:8084/mqtt";
// 如 EMQX公网broker可以是：wss://broker.emqx.io:8084/mqtt
const mqttTopic = "sensor/hrsp";
const clientId = "emqx_vue3_" + Math.random().toString(16).substring(2, 8);

// 模拟历史数据，数据结构保持 { HR: number, SpO2: number, time: Date }
const data = reactive([]);

function connectMqtt() {
  // 连接参数
  const options = {
    clientId: clientId,
    username: "XT",
    password: "lyf20040917",
  };

  client = mqtt.connect(mqttHost, options);

  client.on("connect", () => {
    console.log("MQTT连接成功");
    client.subscribe(mqttTopic, { qos: 0 }, (err) => {
      if (err) {
        console.error("订阅topic失败", err);
      } else {
        console.log(`已订阅主题: ${mqttTopic}`);
      }
    });
  });

  client.on("error", (err) => {
    console.error("MQTT连接错误:", err);
  });

  client.on("offline", () => {
    console.warn("MQTT客户端离线");
  });

  client.on("reconnect", () => {
    console.log("MQTT客户端重连中...");
  });

  client.on("message", (topic, message) => {
    if (topic === mqttTopic) {
      console.log(JSON.parse(message.toString()));

      try {
        const payload = JSON.parse(message.toString());

        let time = payload.time ? new Date(payload.time) : new Date();

        if (data.length >= 100) data.shift();

        data.push({
          HR: payload.HR,
          SpO2: payload.SP02,
          time,
        });

        calcStats();
        updateLineChart();
        updateBarChart();
      } catch (e) {
        console.error("消息解析失败:", e);
      }
    }
  });
}

// refs for charts
const lineChart = ref(null);
const barChart = ref(null);
let lineChartInstance = null;
let barChartInstance = null;

// 生成模拟数据，增加时间戳
function generateData() {
  return {
    HR: Math.floor(40 + Math.random() * 80), // 60~100 bpm
    SpO2: Math.floor(92 + Math.random() * 8), // 92%~100%
    time: new Date(),
  };
}

// 异常阈值判断函数
function isAbnormal(point) {
  return point.HR < 60 || point.HR > 100;
}

// 计算统计数据
const stats = reactive({
  HR: { max: 0, min: 999, avg: 0 },
  SpO2: { max: 0, min: 999, avg: 0 },
});

function calcStats() {
  const hrHighConter = ref(0);
  const hrLowConter = ref(0);
  if (data.length === 0) {
    stats.HR = { max: 0, min: 0, avg: 0 };
    stats.SpO2 = { max: 0, min: 0, avg: 0 };
    return;
  }
  let sumHR = 0,
    sumSpO2 = 0;
  let maxHR = -Infinity,
    minHR = Infinity;
  let maxSpO2 = -Infinity,
    minSpO2 = Infinity;
  data.forEach((d) => {
    sumHR += d.HR;
    sumSpO2 += d.SpO2;
    if (d.HR > maxHR) maxHR = d.HR;
    if (d.HR < minHR) minHR = d.HR;
    if (d.SpO2 > maxSpO2) maxSpO2 = d.SpO2;
    if (d.SpO2 < minSpO2) minSpO2 = d.SpO2;
    if (d.HR > 100) hrHighConter.value++;
    if (d.HR < 60) hrLowConter.value++;
  });
  stats.hrHighConter = hrHighConter.value;
  stats.hrLowConter = hrLowConter.value;

  stats.HR.max = maxHR;
  stats.HR.min = minHR;
  stats.HR.avg = (sumHR / data.length).toFixed(1);
  stats.SpO2.max = maxSpO2;
  stats.SpO2.min = minSpO2;
  stats.SpO2.avg = (sumSpO2 / data.length).toFixed(1);
}

// --- 分页相关数据和计算 ---
const hrPagination = ref({ page: 1, rowsPerPage: 5 });
const spo2Pagination = ref({ page: 1, rowsPerPage: 5 });

// 格式化时间显示
function formatTime(date) {
  return date.toLocaleString();
}

// 心率历史数据分页
const paginatedHRData = computed(() => {
  const start = (hrPagination.value.page - 1) * hrPagination.value.rowsPerPage;
  const end = start + hrPagination.value.rowsPerPage;
  return data
    .map((d, idx) => ({
      id: idx + 1,
      HR: d.HR,
      time: formatTime(d.time),
    }))
    .slice()
    .reverse()
    .slice(start, end);
});

// 血氧历史数据分页
const paginatedSpO2Data = computed(() => {
  const start =
    (spo2Pagination.value.page - 1) * spo2Pagination.value.rowsPerPage;
  const end = start + spo2Pagination.value.rowsPerPage;
  return data
    .map((d, idx) => ({
      id: idx + 1,
      SpO2: d.SpO2,
      time: formatTime(d.time),
    }))
    .slice()
    .reverse()
    .slice(start, end);
});

// 表格列定义
const heartRateColumns = [
  { name: "id", label: "序号", field: "id", align: "center" },
  { name: "time", label: "时间", field: "time", align: "center" },
  { name: "HR", label: "心率 (bpm)", field: "HR", align: "center" },
];
const spo2Columns = [
  { name: "id", label: "序号", field: "id", align: "center" },
  { name: "time", label: "时间", field: "time", align: "center" },
  { name: "SpO2", label: "血氧 (%)", field: "SpO2", align: "center" },
];

// 更新折线图函数
function updateLineChart() {
  if (!lineChartInstance) return;
  const times = data.map((d) => d.time.toLocaleTimeString());
  lineChartInstance.setOption({
    tooltip: {
      trigger: "axis",
      formatter: (params) => {
        if (!params.length) return "";
        let idx = params[0].dataIndex;
        let point = data[idx];
        let timeStr = point.time.toLocaleString();
        let lines = [`${timeStr} `];
        params.forEach((p) => {
          let val = Array.isArray(p.value) ? p.value[1] : p.value;
          lines.push(
            `${p.marker} ${p.seriesName}: ${val}${
              p.seriesName === "心率" ? " bpm" : "%"
            }`
          );
        });
        // 异常提示
        let abnormalNotice = isAbnormal(point)
          ? `<span style="color:red;font-weight:bold;">(异常)</span>`
          : "";
        lines.push(abnormalNotice);
        return lines.join("");
      },
    },
    legend: {
      data: ["心率", "血氧"],
      top: 8,
      textStyle: { fontSize: 12 },
    },
    grid: {
      left: "10%",
      right: "10%",
      bottom: "15%",
      top: "25%",
    },
    xAxis: {
      type: "category",
      boundaryGap: false,
      data: times,
      name: "时间",
      nameLocation: "middle",
      nameGap: 30,
    },
    yAxis: [
      {
        type: "value",
        name: "心率(bpm)",
        min: 40,
        max: 130,
        position: "left",
        offset: 0,
        axisLine: { lineStyle: { color: "#3f51b5" } },
        axisLabel: { formatter: "{value}" },
      },
      {
        type: "value",
        name: "血氧(%)",
        min: 80,
        max: 100,
        position: "right",
        offset: 0,
        axisLine: { lineStyle: { color: "#4caf50" } },
        axisLabel: { formatter: "{value}" },
      },
    ],
    series: [
      {
        name: "心率",
        type: "line",
        yAxisIndex: 0,
        smooth: false,
        data: data.map((d) => d.HR),
        itemStyle: {
          color: "#3f51b5",
        },
        lineStyle: {
          width: 2,
        },
      },
      {
        name: "血氧",
        type: "line",
        yAxisIndex: 1,
        smooth: 0.3,
        data: data.map((d) => d.SpO2),
        itemStyle: {
          color: "#4caf50",
        },
        lineStyle: {
          width: 2,
        },
      },
    ],
  });
}

// 更新血氧柱状图函数
function updateBarChart() {
  if (!barChartInstance) return;
  const times = data.map((d) => d.time.toLocaleTimeString());
  barChartInstance.setOption({
    tooltip: {
      trigger: "axis",
      axisPointer: { type: "shadow" },
      formatter: (params) => {
        if (!params.length) return "";
        let idx = params[0].dataIndex;
        let point = data[idx];
        return `${point.time.toLocaleString()}  血氧: ${params[0].data} %`;
      },
    },
    grid: {
      left: "12%",
      right: "10%",
      bottom: "15%",
      top: "20%",
    },
    xAxis: {
      type: "category",
      data: times,
      name: "时间",
      nameLocation: "middle",
      nameGap: 28,
      axisLabel: { rotate: 0 },
    },
    yAxis: {
      type: "value",
      min: 80,
      max: 100,
      name: "血氧(%)",
      axisLabel: { formatter: "{value}" },
    },
    series: [
      {
        name: "血氧",
        type: "bar",
        barWidth: "60%",
        data: data.map((d) => d.SpO2),
        itemStyle: {
          color: "#4caf50",
        },
      },
    ],
  });
}

onMounted(() => {
  lineChartInstance = echarts.init(lineChart.value);
  barChartInstance = echarts.init(barChart.value);
  connectMqtt();

  // 先推入10条初始数据
  // for (let i = 0; i < 10; i++) {
  //   data.push(generateData());
  // }
  // calcStats();
  // updateLineChart();
  // updateBarChart();
  // 模拟实时数据每秒监听
  // const timer = setInterval(() => {
  //   data.push(generateData());
  //   if (data.length > 100) data.shift(); // 保留100条历史数据，你可调整
  //   calcStats();
  //   updateLineChart();
  //   updateBarChart();
  // }, 1000);

  // onBeforeUnmount(() => {
  //   clearInterval(timer);
  //   if (lineChartInstance) lineChartInstance.dispose();
  //   if (barChartInstance) barChartInstance.dispose();
  // });
});

onBeforeUnmount(() => {
  if (client) {
    client.end(true);
  }
  if (lineChartInstance) lineChartInstance.dispose();
  if (barChartInstance) barChartInstance.dispose();
});
</script>

<style scoped>
/* 常规卡片样式调整，保持简洁美观 */
.q-card {
  box-shadow: #e0e0e0 0px 3px 8px;
  border-radius: 8px;
  background-color: white;
}
</style>
