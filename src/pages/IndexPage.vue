<template>
  <q-page padding>
    <q-card>
      <q-card-section>
        <div ref="chart" style="width: 100%; height: 350px"></div>
      </q-card-section>
    </q-card>
  </q-page>
</template>

<script setup>
import { ref, onMounted, onBeforeUnmount } from "vue";
import * as echarts from "echarts";

const chart = ref(null);
let myChart = null;

const data = [];

function generateData() {
  const now = new Date();
  return {
    time: now,
    HR: Math.floor(60 + Math.random() * 40), // 60～100 bpm
    SpO2: Math.floor(92 + Math.random() * 8), // 92%～100%
  };
}

function isAbnormal(point) {
  return point.HR < 50 || point.HR > 120 || point.SpO2 < 90;
}

function updateChart() {
  if (!myChart) return;

  myChart.setOption({
    tooltip: {
      trigger: "axis",
      formatter: function (params) {
        if (!params || !params.length) return "";
        const time = params[0].axisValueLabel; // 时间字符串
        let tooltipText = time + "<br/>";
        params.forEach((p) => {
          // p.value 是数组，第二个元素是数值
          let val = Array.isArray(p.value) ? p.value[1] : p.value;
          tooltipText += `${p.marker} ${p.seriesName}: ${val}${
            p.seriesName === "心率" ? " bpm" : "%"
          }<br/>`;
        });
        return tooltipText;
      },
    },
    xAxis: {
      type: "category",
      boundaryGap: false,
      data: data.map((item) => item.time.toLocaleTimeString()),
    },
    yAxis: [
      { type: "value", name: "心率（bpm）", min: 40, max: 130 },
      { type: "value", name: "血氧（%）", min: 80, max: 100 },
    ],
    series: [
      {
        name: "心率",
        type: "line",
        yAxisIndex: 0,
        data: data.map((item) => ({
          value: [item.time.toLocaleTimeString(), item.HR],
          itemStyle: isAbnormal(item) ? { color: "red" } : {},
        })),
        smooth: 0.1,
        showSymbol: false,
      },
      {
        name: "血氧",
        type: "line",
        yAxisIndex: 1,
        data: data.map((item) => ({
          value: [item.time.toLocaleTimeString(), item.SpO2],
          itemStyle: isAbnormal(item) ? { color: "red" } : {},
        })),
        smooth: 0.2,
        showSymbol: false,
      },
    ],
  });
}

onMounted(() => {
  myChart = echarts.init(chart.value);

  for (let i = 0; i < 10; i++) {
    data.push(generateData());
  }
  updateChart();

  const interval = setInterval(() => {
    data.push(generateData());
    if (data.length > 20) data.shift();
    updateChart();
  }, 1000);

  onBeforeUnmount(() => {
    clearInterval(interval);
    myChart.dispose();
  });
});
</script>
