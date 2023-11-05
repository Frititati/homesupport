# Use an official lightweight Node.js image
FROM node:alpine

# Set the working directory inside the container
WORKDIR /app

# Copy package.json and package-lock.json
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy the rest of the app's source code from your host to your image filesystem.
COPY . .

# React uses port 3000 by default
EXPOSE 3000

# Configure the container to run in an executed manner
ENTRYPOINT ["npm", "start"]
