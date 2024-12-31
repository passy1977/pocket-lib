


#[allow(unused)] 
#[derive(Debug)]
pub struct Config {
    config_path: String,
    pub init: bool
}

#[allow(unused)] 
impl Config {
    pub fn new(config_path: Option<String>) -> Config {
        if let Some(unwap_config_path) = config_path {
            let mut config = Config{
                config_path : unwap_config_path,
                init: false
            };




            config.init = true;
            config
        } else {
            Config{
                config_path : "".to_string(),
                init: false
            }
        }
    }
}
