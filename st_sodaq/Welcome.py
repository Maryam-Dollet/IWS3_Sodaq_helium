import streamlit as st
from load import load_sensor_data

st.title("Helium IWS3 Data")

sensor_df = load_sensor_data()

st.dataframe(sensor_df)