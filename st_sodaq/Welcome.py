import streamlit as st
import pandas as pd
import plotly.express as px
from load import load_sensor_data
from time import sleep

st.set_page_config(
    page_title = 'IWS3 Data Dashboard',
    page_icon = ':ear_of_rice:',
    layout= "wide"
)

st.title("Helium IWS3 Data Dashboard")

placeholder = st.empty()

for seconds in range(500):

    sensor_df = load_sensor_data().iloc[:, : 9]

    sensor_df['time'] = pd.to_datetime(sensor_df['Horodateur'], format='%m/%d/%Y %H:%M:%S') 

    with placeholder.container():

        with st.expander("Data"):
            st.dataframe(sensor_df, use_container_width=True)
        
        if sensor_df.tail(1).moisture.values[0] >= 500:
            st.write("Actual State of the Soil: Wet")
        else:
            st.write("Actual State of the Soil: Dry")

        col1, col2 = st.columns(2)

        with col1:
            fig = px.line(sensor_df, x='time', y="temperature", range_y=[0,38], title="Temperature Measures", labels={"temperature":"temperature ( °C )"}, markers=True)
            st.plotly_chart(fig)

            fig = px.line(sensor_df, x='time', y="humidity", range_y=[0,100], title="Humidity Measures", labels={"humidity":"humidity ( % )"}, markers=True)
            st.plotly_chart(fig)

            fig = px.line(sensor_df, x='time', y="gas", range_y=[0,1000], title="Gas Measures", labels={"gas":"gas ( KOhms )"}, markers=True)
            st.plotly_chart(fig)

        with col2:
            fig = px.line(sensor_df, x='time', y="pressure", range_y=[900,1100], title="Pressure Measures", labels={"pressure":"pressure ( hPa )"}, markers=True)
            st.plotly_chart(fig)

            fig = px.line(sensor_df, x='time', y="dewPoint", range_y=[0,30], title="Dew Point Measures", labels={"dewPoint":"dew point ( °C )"}, markers=True)
            st.plotly_chart(fig)

            fig = px.line(sensor_df, x='time', y="moisture", range_y=[0,1010], title="Soil Moisture Measures", labels={"moisture":"soil moisture"}, markers=True)
            st.plotly_chart(fig)
            
            sleep(40)

