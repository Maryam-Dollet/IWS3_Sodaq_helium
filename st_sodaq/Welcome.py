import streamlit as st
import plotly.express as px
from load import load_sensor_data
from time import sleep

st.set_page_config(
    page_title = 'IWS3 Data Dashboard',
    page_icon = ':ear_of_rice:',
)

st.title("Helium IWS3 Data Dashboard")

sensor_df = load_sensor_data()

with st.expander("Data"):
    st.dataframe(sensor_df, use_container_width=True)

fig = px.line(sensor_df, x='Horodateur', y="temperature", range_y=[0,38])
st.plotly_chart(fig)

fig = px.line(sensor_df, x='Horodateur', y="pressure", range_y=[0,1100])
st.plotly_chart(fig)

fig = px.line(sensor_df, x='Horodateur', y="humidity", range_y=[0,100])
st.plotly_chart(fig)

fig = px.line(sensor_df, x='Horodateur', y="dewPoint", range_y=[0,100])
st.plotly_chart(fig)

fig = px.line(sensor_df, x='Horodateur', y="altitude", range_y=[0,500])
st.plotly_chart(fig)

fig = px.line(sensor_df, x='Horodateur', y="gas", range_y=[0,1000])
st.plotly_chart(fig)

fig = px.line(sensor_df, x='Horodateur', y="moisture", range_y=[0,1010])
st.plotly_chart(fig)

sleep(10000)
