import setuptools
setuptools.setup(
    name="binary_configurator",
    version="3.4.0",
    author="LoraMesh Team",
    author_email="",
    description="LoraMesh Binary Installer",
    long_description='LoraMesh binary configurator and flasher tool all-in-one',
    long_description_content_type="text/markdown",
    url="https://github.com/LoraMesh/LoraMesh",
    packages=['.'] + setuptools.find_packages(),
    include_package_data=True,
    entry_points={
        "console_scripts": ["flash=binary_configurator:main"],
    },
    install_requires=['pyserial'],
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)
