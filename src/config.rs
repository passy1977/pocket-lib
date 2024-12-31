use crate::pods::device::Device;
use std::env;



#[derive(Debug)]
pub struct Config {
    config_path: Option<String>,
    pub device: Option<Device>,
    pub init: bool
}

impl Config {

    pub fn new(config_json: String, config_path: Option<String>) -> Option<Config> {

        let mut absolute_path = String::new();
        if let Some(path) = config_path {
            absolute_path = env::var("HOME").unwrap();
        } else {
            absolute_path = config_path.unwrap();
        }


        if let Ok(parsed) = json::parse(config_json.as_str()) {
            let mut config = Config{
                config_path: Option::None,
                device: Option::None,
                init: false
            };

            let mut device = Device::new();

            if let Some(uuid) = parsed["uuid"].as_str() {
                device.uuid = uuid.to_string();
            } else {
                return None
            }
            

            if let Some(user_uuid) = parsed["user_uuid"].as_str() {
                device.user_uuid = user_uuid.to_string();
            } else {
                return None
            }

            if let Some(host) = parsed["host"].as_str() {
                device.host = host.to_string();
            } else {
                return None
            }

            if let Some(host_pub_key) = parsed["host_pub_key"].as_str() {
                device.host_pub_key = host_pub_key.to_string();
            } else {
                return None
            }

            config.config_path = Some(absolute_path);
            config.store();
            config.init = true;
            
            Option::Some(config)
        } else {
            None
        }
    }

    fn store(&self) {

    }
}
