#!/bin/bash

wget https://download1.rstudio.org/electron/jammy/amd64/rstudio-2023.12.1-402-amd64.deb -O rstudio-2023.12.1-402-amd64.deb && \
sudo chmod +x rstudio-2023.12.1-402-amd64.deb && \
sudo dpkg -i rstudio-2023.12.1-402-amd64.deb